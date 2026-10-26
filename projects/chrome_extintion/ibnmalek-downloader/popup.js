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
      'افتح صفحة المقرر أو الدرس على موقع معهد ابن مالك أولًا.';
    return;
  }

  $('scanBtn').disabled = true;
  $('pageNotice').textContent = 'جاري قراءة الصفحة الحالية…';

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
      throw new Error(payload?.error || 'تعذر قراءة الصفحة');
    }

    const res = await bg({
      type: 'COLLECT_RESULT',
      payload
    });

    const type =
      payload.kind === 'lesson'
        ? 'صفحة درس'
        : 'صفحة مقرر';

    $('pageNotice').textContent =
      `تم فحص ${type}: ${payload.pageTitle || tab.url}. ` +
      `جديد: ${res.result.added}، ` +
      `متغير: ${res.result.changed}، ` +
      `دون تغيير: ${res.result.unchanged}.`;

    await refresh();
  } catch (e) {
    $('pageNotice').textContent =
      'تعذر جمع البيانات: ' + e.message;
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
    task.current || 'لا توجد مهمة قيد التشغيل.';

  const pill = $('statusPill');
  const status = task.status || 'idle';

  pill.textContent = ({
    idle: 'جاهز',
    running: 'جارٍ العمل',
    done: 'اكتمل',
    'done-with-errors': 'اكتمل مع أخطاء'
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
      ? 'آخر حفظ: ' + fmtTime(s.db.meta.updatedAt)
      : 'البيانات محفوظة على هذا المتصفح';

  const logs = s.logs || [];

  $('logList').innerHTML = '';

  if (!logs.length) {
    $('logList').innerHTML =
      '<li class="empty">ستظهر هنا خطوات الإضافة ونتائجها.</li>';
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
    '&': '&amp;',
    '<': '&lt;',
    '>': '&gt;',
    '"': '&quot;',
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
        'إعادة فتح كل صفحات الدروس حتى لو كانت بياناتها حديثة؟ قد يستغرق ذلك وقتًا أطول.'
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
      'افتح صفحة المقرر أو الدرس في معهد ابن مالك.';
  }

  await refresh();

  setInterval(refresh, 1200);
})();