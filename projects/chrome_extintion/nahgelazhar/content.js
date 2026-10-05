(function () {
  'use strict';

  const KEY = 'nahj_state';
  const log = (...a) => console.log('[Nahj Helper]', ...a);
  const norm = (s) => (s || '').replace(/\s+/g, ' ').trim();
  const sleep = (ms) => new Promise((r) => setTimeout(r, ms));

  let restoring = false;

  const getUnits = () =>
    Array.from(document.querySelectorAll('button')).filter((b) => b.querySelector('h3'));
  const isOpen = (unit) => !!unit.querySelector(':scope > svg.rotate-180');
  const unitTitle = (unit) => norm(unit.querySelector('h3').textContent);
  const openUnits = () => getUnits().filter(isOpen);

  function waitFor(fn, timeout = 1500) {
    return new Promise((resolve) => {
      const t0 = Date.now();
      (function tick() {
        const r = fn();
        if (r) return resolve(r);
        if (Date.now() - t0 > timeout) return resolve(null);
        setTimeout(tick, 50);
      })();
    });
  }

  // حفظ الوحدة المفتوحة + المسافة من أول الصفحة (بس لما تكون وحدة واحدة مفتوحة)
  function save() {
    if (restoring) return;
    const open = openUnits();
    if (open.length !== 1) return;
    const data = { unit: unitTitle(open[0]), y: Math.round(window.scrollY) };
    try { localStorage.setItem(KEY, JSON.stringify(data)); } catch (_) {}
  }

  let scrollTimer = null;
  window.addEventListener('scroll', () => {
    clearTimeout(scrollTimer);
    scrollTimer = setTimeout(save, 150);
  }, { passive: true });

  // كليكات المستخدم الحقيقية بس
  document.addEventListener('click', (e) => {
    if (!e.isTrusted) return;
    const btn = e.target.closest('button');
    if (!btn) return;

    // كليك على وحدة: لو هتتفتح، اقفل الباقي
    if (btn.querySelector('h3')) {
      if (!isOpen(btn)) {
        setTimeout(() => {
          getUnits().forEach((u) => { if (u !== btn && isOpen(u)) u.click(); });
          setTimeout(() => {
            btn.style.scrollMarginTop = '140px';
            btn.scrollIntoView({ behavior: 'smooth', block: 'start' });
          }, 100);
        }, 30);
      }
      return;
    }

    // كليك على درس: احفظ المكان فوراً قبل ما الصفحة تتغير
    if (btn.querySelector('h4')) save();
  }, true);

  window.addEventListener('pagehide', save);

  async function restore(units) {
    let saved = null;
    try { saved = JSON.parse(localStorage.getItem(KEY)); } catch (_) {}

    const keep = saved ? units.find((u) => unitTitle(u) === saved.unit) : null;

    // اقفل كل الوحدات ما عدا المحفوظة
    let n = 0;
    units.forEach((u) => { if (u !== keep && isOpen(u)) { u.click(); n++; } });
    log(`Units: ${units.length}, collapsed: ${n}`);

    if (!keep) return;

    restoring = true;
    if (!isOpen(keep)) keep.click();

    // استنى لحد ما يفضل وحدة واحدة مفتوحة ودروسها ظهرت
    await waitFor(() => openUnits().length === 1 && keep.parentElement.querySelector(':scope > div button'));
    await sleep(150);

    window.scrollTo({ top: saved.y || 0, behavior: 'auto' });
    log('Restored:', saved);

    await sleep(500);
    restoring = false;
  }

  function process() {
    const units = getUnits();
    if (!units.length) return;

    const host = units[0].parentElement.parentElement;
    if (host.dataset.nahjDone) return;
    host.dataset.nahjDone = '1';

    restore(units);
  }

  let timer = null;
  new MutationObserver(() => {
    clearTimeout(timer);
    timer = setTimeout(process, 300);
  }).observe(document.documentElement, { childList: true, subtree: true });

  process();
})();