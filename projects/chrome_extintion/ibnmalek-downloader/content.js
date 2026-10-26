(() => {
  if (window.__ibnMalekCollectorLoaded) return;
  window.__ibnMalekCollectorLoaded = true;

  const clean = s => (s || '').replace(/\s+/g, ' ').trim();

  const absolute = href => {
    try {
      return new URL(href, location.href).href;
    } catch {
      return '';
    }
  };

  const youtubeId = text => {
    const m = String(text || '').match(
      /(?:youtube\.com\/(?:embed|shorts|live)\/|youtu\.be\/|youtube\.com\/vi\/|youtube\.com\/watch\?v=)([\w-]{11})/
    );

    return m ? m[1] : '';
  };

  const safeFileUrl = href => {
    try {
      const u = new URL(href, location.href);
      return u.protocol === 'https:' || u.protocol === 'http:';
    } catch {
      return false;
    }
  };

  function collectCourse() {
    const links = [...document.querySelectorAll('a[href*="lesson_id="]')];
    const byId = new Map();

    for (const a of links) {
      const href = absolute(a.getAttribute('href'));
      if (!href) continue;

      const u = new URL(href);
      const id = u.searchParams.get('lesson_id');
      if (!id) continue;

      const card =
        a.closest('.q-card, .q-px-xs, article, li') ||
        a.parentElement?.parentElement ||
        a;

      const titleNode =
        card.querySelector(
          '.text-primary.text-bold, .text-caption.text-bold, h1, h2, h3'
        ) || a;

      const title = clean(titleNode.textContent) || `درس ${id}`;
      const numberMatch = title.match(/^\s*(\d{1,3})\s+/);

      const teacherText = clean(card.textContent).match(
        /(?:شرح الشيخ|المحاضر)\s*[:：]?\s*([^\n]{2,80})/
      );

      const thumb = card.querySelector(
        'img[src], [style*="youtube.com/vi/"]'
      );

      const thumbText =
        `${thumb?.getAttribute('src') || ''} ` +
        `${thumb?.getAttribute('style') || ''} ` +
        card.innerHTML.slice(0, 5000);

      const ytId = youtubeId(thumbText);
      const teacher = teacherText ? teacherText[1].trim() : '';
      const key = `${location.origin}|${id}`;

      const old = byId.get(key);

      if (!old || title.length > old.title.length) {
        byId.set(key, {
          id,
          key,
          title,
          number: numberMatch
            ? numberMatch[1].padStart(2, '0')
            : '',
          teacher,
          lessonUrl: href,
          youtubeId: ytId,
          youtubeUrl: ytId
            ? `https://www.youtube.com/watch?v=${ytId}`
            : '',
          sourcePage: location.href,
          scannedAt: new Date().toISOString(),
          attachments: [],
          exercisesHtml: '',
          fingerprint: '',
          lastCheckedAt: '',
          status: 'discovered'
        });
      }
    }

    return [...byId.values()];
  }

  async function collectLesson() {
    const cleanLabel = s =>
      clean(s)
        .replace(/fiber_manual_record|file_download|description/g, '')
        .trim();

    // فتح تبويب المرفقات حتى تظهر عناصره قبل جمع الروابط.
    const attachmentTab = [...document.querySelectorAll('[role="listitem"]')]
      .find(el => /^مرفقات$/.test(clean(el.innerText)));

    if (attachmentTab && !attachmentTab.classList.contains('q-item--active')) {
      attachmentTab.click();
      await new Promise(resolve => setTimeout(resolve, 800));
    }

    const attachments = [];
    const seen = new Set();

    for (const a of document.querySelectorAll('a[href]')) {
      const href = absolute(a.getAttribute('href'));

      if (
        !href ||
        !safeFileUrl(href) ||
        href.startsWith('javascript:') ||
        href.startsWith('mailto:') ||
        href.includes('youtube.com/watch') ||
        seen.has(href)
      ) continue;

      const row = a.closest('[role="listitem"]');
      const rowText = cleanLabel(row?.innerText || '');
      const linkText = cleanLabel(
        a.getAttribute('download') ||
        a.innerText ||
        a.textContent ||
        ''
      );

      const label = rowText || linkText || decodeURIComponent(
        new URL(href).pathname.split('/').pop() || 'ملف'
      );

      const lower = `${href} ${label}`.toLowerCase();

      const looksFile =
        /\.(pdf|mp3|m4a|aac|wav|ogg|mp4|m4v|webm|docx?|xlsx?|pptx?|zip|rar|png|jpe?g)(?:$|[?#])/i.test(href) ||
        /الدرس صوتي|مشجرات الدرس|تفريغ الدرس|تحميل|download|audio|pdf|مرفق|ملف/.test(lower);

      if (!looksFile) continue;

      seen.add(href);

      attachments.push({
        url: href,
        label: label.slice(0, 160),
        kind: inferKind(href, label),
        filename: ''
      });
    }

    const exerciseHeading = [
      ...document.querySelectorAll(
        'h1,h2,h3,h4,.text-subtitle1,.text-h6'
      )
    ].find(el =>
      /التمارين|الأسئلة|التدريبات/i.test(clean(el.textContent))
    );

    let exercisesHtml = '';

    if (exerciseHeading) {
      const root = exerciseHeading.parentElement;
      const clone = root?.cloneNode(true);

      if (clone) {
        clone.querySelectorAll(
          'script,style,button,nav,header,footer,iframe'
        ).forEach(el => el.remove());

        exercisesHtml = clone.innerHTML.slice(0, 250000);
      }
    }

    const videoIds = [
      ...new Set(
        [...document.querySelectorAll(
          'iframe[src],a[href],img[src],[style*="youtube.com/vi/"]'
        )]
          .map(el =>
            `${el.src || el.href || ''} ${el.getAttribute?.('style') || ''}`
          )
          .map(youtubeId)
          .filter(Boolean)
      )
    ];

    return {
      attachments,
      exercisesHtml,
      videoIds,
      scannedAt: new Date().toISOString(),
      pageUrl: location.href,
      pageTitle: clean(document.title)
    };
  }

  function inferKind(url, label) {
    const s = `${url} ${label}`.toLowerCase();

    if (
      /\.(mp3|m4a|aac|wav|ogg)(?:$|[?#])|صوت|audio/.test(s)
    ) {
      return 'audio';
    }

    if (
      /\.pdf(?:$|[?#])|مشجر|تفريغ|pdf/.test(s)
    ) {
      return 'document';
    }

    if (
      /\.(mp4|m4v|webm)(?:$|[?#])/.test(s)
    ) {
      return 'video-direct';
    }

    return 'attachment';
  }

  chrome.runtime.onMessage.addListener((msg, sender, sendResponse) => {
    if (msg?.type === 'COLLECT_PAGE') {
      (async () => {
        const isLesson =
          /lesson_id=/.test(location.search) ||
          /\/lesson\b/.test(location.pathname);

        sendResponse({
          ok: true,
          kind: isLesson ? 'lesson' : 'course',
          lessons: isLesson ? [] : collectCourse(),
          detail: isLesson ? await collectLesson() : null,
          pageUrl: location.href,
          pageTitle: clean(document.title)
        });
      })().catch(error => {
        sendResponse({ ok: false, error: String(error) });
      });

      return true;
    }

    if (msg?.type === 'GET_PAGE_INFO') {
      sendResponse({
        ok: true,
        pageUrl: location.href,
        pageTitle: clean(document.title)
      });
    }

    return true;
  });
})();