const $ = id => document.getElementById(id);

let activeTab = null;

const fmtTime = s => {
  try {
    return new Date(s).toLocaleTimeString('ar-EG', {
      hour: '2-digit',
      minute: '2-digit',
      second: '2-digit'
    });
  } catch {
    return '';
  }
};

async function bg(message) {
  return chrome.runtime.sendMessage(message);
}

async function active() {
  const tabs = await chrome.tabs.query({
    active: true,
    currentWindow: true
  });

  return tabs[0];
}

async function collectCurrent() {
  const tab = await active();

  if (
    !tab?.id ||
    !/^https:\/\/(?:[^/]+\.)?(?:ibnmalekar\.com|tamayyuzcenter\.com)\//.test(
      tab.url || ''
    )
  ) {
    $('pageNotice').textContent =
      'Open a course or lesson page on the Ibn Malek website first.';
    return;
  }

  $('scanBtn').disabled = true;
  $('pageNotice').textContent = 'Reading the current page…';

  try {
    const payload = await chrome.tabs
      .sendMessage(tab.id, {
        type: 'COLLECT_PAGE'
      })
      .catch(async () => {
        await chrome.scripting.executeScript({
          target: {
            tabId: tab.id
          },
          files: ['content.js']
        });

        return chrome.tabs.sendMessage(tab.id, {
          type: 'COLLECT_PAGE'
        });
      });

    if (!payload?.ok) {
      throw new Error(payload?.error || 'Could not read the page');
    }

    const res = await bg({
      type: 'COLLECT_RESULT',
      payload
    });

    const type =
      payload.kind === 'lesson'
        ? 'Lesson page'
        : 'Course page';

    $('pageNotice').textContent =
      `Checked ${type}: ${payload.pageTitle || tab.url}. ` +
      `New: ${res.result.added}, ` +
      `Changed: ${res.result.changed}, ` +
      `Unchanged: ${res.result.unchanged}.`;

    await refresh();
  } catch (e) {
    $('pageNotice').textContent =
      'Could not collect data: ' + e.message;
  } finally {
    $('scanBtn').disabled = false;
  }
}

async function refresh() {
  const s = await bg({
    type: 'GET_STATE'
  });

  if (!s?.ok) return;

  const lessons = Object.values(s.db.lessons || {});

  const files = lessons.reduce(
    (n, l) => n + (l.attachments || []).length,
    0
  );

  const task = s.task || {};

  const noMedia = lessons.filter(l =>
    l.detailFingerprint &&
    !(l.attachments || []).length &&
    !(l.videoIds || []).length &&
    !l.youtubeId &&
    !l.youtubeUrl
  ).length;

  $('lessonCount').textContent = lessons.length;
  $('fileCount').textContent = files;

  $('doneCount').textContent = task.done || 0;

  const finished = [
    'done',
    'done-with-errors'
  ].includes(task.status);

  $('remainingCount').textContent =
    task.status === 'running'
      ? Math.max(
          0,
          (task.total || 0) - (task.done || 0)
        )
      : finished
        ? 0
        : Math.max(0, task.remaining || 0);

  $('noMediaCount').textContent = noMedia;

  const pct = task.total
    ? Math.min(
        100,
        Math.round((task.done || 0) / task.total * 100)
      )
    : 0;

  $('progressBar').style.width = pct + '%';

  $('currentTask').textContent =
    task.current || 'No task is running.';

  const pill = $('statusPill');
  const status = task.status || 'idle';

  pill.textContent = ({
    idle: 'Ready',
    running: 'Running',
    done: 'Completed',
    'done-with-errors': 'Completed with errors'
  })[status] || status;

  pill.className =
    'pill ' +
    (
      status === 'running'
        ? 'running'
        : status === 'done'
          ? 'done'
          : status === 'done-with-errors'
            ? 'error'
            : 'idle'
    );

  $('lastSaved').textContent =
    s.db.meta?.updatedAt
      ? 'Last saved: ' + fmtTime(s.db.meta.updatedAt)
      : 'Data is saved in this browser';

  const logs = s.logs || [];

  $('logList').innerHTML = '';

  if (!logs.length) {
    $('logList').innerHTML =
      '<li class="empty">Extension steps and results will appear here.</li>';
  }

  for (const row of logs.slice(0, 35)) {
    const li = document.createElement('li');

    li.innerHTML =
      `<span class="time">${fmtTime(row.time)}</span> ` +
      escapeHtml(row.message);

    if (row.level === 'error') {
      li.style.color = '#b42318';
    }

    $('logList').appendChild(li);
  }
}

function escapeHtml(s) {
  return String(s).replace(/[&<>"']/g, c => ({
    '&': '&',
    '<': '<',
    '>': '>',
    '"': '"',
    "'": '&#39;'
  })[c]);
}

$('scanBtn').addEventListener(
  'click',
  collectCurrent
);

$('batchScanBtn').addEventListener(
  'click',
  async () => {
    await bg({
      type: 'BATCH_SCAN',
      force: false
    });

    await refresh();
  }
);

$('forceScanBtn').addEventListener(
  'click',
  async () => {
    if (
      confirm(
        'Reopen every lesson page even if its data is current? This may take longer.'
      )
    ) {
      await bg({
        type: 'BATCH_SCAN',
        force: true
      });

      await refresh();
    }
  }
);

$('printBtn').addEventListener(
  'click',
  async () => {
    await bg({
      type: 'PRINT_EXERCISES'
    });

    await refresh();
  }
);

$('downloadBtn').addEventListener(
  'click',
  async () => {
    await bg({
      type: 'DOWNLOAD_ATTACHMENTS'
    });

    await refresh();
  }
);

$('exportBtn').addEventListener(
  'click',
  async () => {
    await bg({
      type: 'EXPORT_CSV'
    });

    await refresh();
  }
);

$('clearLogBtn').addEventListener(
  'click',
  async () => {
    await bg({
      type: 'CLEAR_LOG'
    });

    await refresh();
  }
);

$('openSiteBtn').addEventListener(
  'click',
  () => chrome.tabs.create({
    url: 'https://ibnmalekar.com/usercontrol'
  })
);

$('parallel').addEventListener(
  'change',
  saveSettings
);

$('quality').addEventListener(
  'change',
  saveSettings
);

async function saveSettings() {
  await bg({
    type: 'SAVE_SETTINGS',
    settings: {
      parallel: Number($('parallel').value),
      quality: $('quality').value
    }
  });
}

(async () => {
  const s = await bg({
    type: 'GET_STATE'
  });

  $('parallel').value = String(
    s.settings?.parallel || 3
  );

  $('quality').value =
    s.settings?.quality || 'best';

  activeTab = await active();

  if (
    !activeTab?.url?.includes('ibnmalekar.com') &&
    !activeTab?.url?.includes('tamayyuzcenter.com')
  ) {
    $('pageNotice').textContent =
      'Open a course or lesson page on the Ibn Malek website.';
  }

  await refresh();

  setInterval(refresh, 1200);
})();
