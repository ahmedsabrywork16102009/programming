(function () {
  'use strict';

  const KEY = 'nahj_state';
  const LAST_KEY = 'nahj_last_unit';
  const SETTINGS_KEY = 'nahj_settings';
  const CACHE_KEY = 'nahj_stats_cache';
  const FAB_KEY = 'nahj_fab_side';
  const log = (...a) => console.log('[Nahj Helper]', ...a);
  const norm = (s) => (s || '').replace(/\s+/g, ' ').trim();
  const sleep = (ms) => new Promise((r) => setTimeout(r, ms));
  const read = (k, d) => { try { const v = JSON.parse(localStorage.getItem(k)); return v === null || v === undefined ? d : v; } catch (_) { return d; } };
  const write = (k, v) => { try { localStorage.setItem(k, JSON.stringify(v)); } catch (_) {} };

  if ('scrollRestoration' in history) history.scrollRestoration = 'manual';

  let restoring = true; // مقفول لحد ما الاسترجاع يخلص
  let settings = Object.assign({ hide: false, exclude: false }, read(SETTINGS_KEY, {}));
  const cache = read(CACHE_KEY, {});

  // ---------- أدوات الوحدات ----------
  const getUnits = () =>
    Array.from(document.querySelectorAll('button')).filter((b) => b.querySelector('h3'));
  const isOpen = (unit) => !!unit.querySelector(':scope > svg.rotate-180');
  const unitTitle = (unit) => norm(unit.querySelector('h3').textContent);
  const openUnits = () => getUnits().filter(isOpen);
  const isLit = (title) => title.includes('الأدبي');
  const unitCard = (unit) => unit.parentElement;
  const isVisible = (unit) => unitCard(unit).offsetParent !== null;
  const findUnit = (title) => getUnits().find((u) => unitTitle(u) === title && isVisible(u));

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

  // ---------- تصنيف الدروس ----------
  function classify(unitName, name, type) {
    const tags = [];
    if (type === 'فيديو') {
      tags.push('video');
      tags.push(name.includes('حل') || name.includes('واجب') ? 'hw' : 'explain');
    } else if (type === 'ملف') {
      if (name.includes('سبورة')) tags.push('board');
      else if (name.includes('واجب') || name.includes('حل')) tags.push('hw');
    } else if (type === 'اختبار') {
      if (unitName.includes('شامل') || name.includes('شامل')) tags.push('comp');
      else if (name.includes('قبل')) tags.push('pre');
      else if (name.includes('بعد')) tags.push('post');
      else tags.push('other');
    }
    return tags;
  }

  function collect() {
    getUnits().forEach((unit) => {
      const title = unitTitle(unit);
      const m = unit.textContent.match(/(\d+)\s*\/\s*(\d+)\s*دروس/);
      const entry = cache[title] || { lessons: [] };
      if (m) { entry.done = +m[1]; entry.total = +m[2]; }
      const box = unitCard(unit).querySelector(':scope > div');
      const btns = box ? Array.from(box.querySelectorAll(':scope > button')) : [];
      if (btns.length) {
        entry.lessons = btns.map((b) => {
          const h4 = b.querySelector('h4');
          const img = b.querySelector('img');
          const name = norm(h4 && h4.textContent);
          const type = norm(img && img.alt);
          const done = Array.from(b.querySelectorAll('span')).some((s) => norm(s.textContent) === 'مكتمل');
          return { tags: classify(title, name, type), done };
        });
      }
      cache[title] = entry;
    });
    write(CACHE_KEY, cache);
  }

  function computeStats() {
    const s = {};
    ['units', 'video', 'explain', 'board', 'hw', 'pre', 'post', 'comp', 'other'].forEach((k) => (s[k] = { d: 0, t: 0 }));
    getUnits().forEach((unit) => {
      const title = unitTitle(unit);
      if (isLit(title) && (settings.hide || settings.exclude)) return;
      const e = cache[title];
      if (!e) return;
      if (e.total) { s.units.t++; if (e.done === e.total) s.units.d++; }
      (e.lessons || []).forEach((l) => l.tags.forEach((t) => { s[t].t++; if (l.done) s[t].d++; }));
    });
    return s;
  }

  // ---------- كارت الإحصائيات ----------
  const CARDS = [
    ['units', '📚', 'الوحدات المكتملة'],
    ['video', '🎬', 'كل الفيديوهات'],
    ['explain', '🎓', 'فيديوهات الشرح'],
    ['board', '📝', 'ملفات السبورة'],
    ['hw', '✍️', 'الواجبات وحلولها'],
    ['pre', '🔎', 'الاختبارات القبلية'],
    ['post', '✅', 'الاختبارات البعدية'],
    ['comp', '🏆', 'الاختبارات الشاملة'],
    ['other', '🧩', 'اختبارات أخرى'],
  ];

  const bar = (p, h) =>
    `<div class="overflow-hidden rounded-full bg-white/15" style="height:${h}px"><div class="h-full rounded-full bg-gold-400 transition-all duration-700" style="width:${p}%"></div></div>`;

  function render() {
    const card = document.getElementById('nahj-stats');
    if (!card) return;
    const s = computeStats();
    const parts = CARDS.filter((c) => c[0] !== 'units' && c[0] !== 'video');
    const all = parts.reduce((a, c) => a + s[c[0]].t, 0);
    const allDone = parts.reduce((a, c) => a + s[c[0]].d, 0);
    const pct = (o) => (o.t ? Math.round((o.d / o.t) * 100) : 0);

    const items = CARDS.filter((c) => s[c[0]].t > 0 || c[0] !== 'other').map(([k, icon, label]) => {
      const o = s[k];
      return `<div style="background:rgba(255,255,255,.08);border:1px solid rgba(255,255,255,.14);border-radius:16px;padding:12px 14px;color:#fff">
        <div style="display:flex;align-items:center;gap:8px;margin-bottom:8px">
          <span style="width:32px;height:32px;border-radius:10px;background:rgba(224,184,74,.25);display:flex;align-items:center;justify-content:center;font-size:16px">${icon}</span>
          <span style="font-size:13px;font-weight:700">${label}</span>
        </div>
        <div style="display:flex;align-items:baseline;justify-content:space-between;margin-bottom:8px">
          <span style="font-size:20px;font-weight:800">${o.d} <span style="font-size:13px;opacity:.7;font-weight:600">/ ${o.t}</span></span>
          <span style="font-size:12px;font-weight:700;color:#f3d77a">${pct(o)}%</span>
        </div>
        ${bar(pct(o), 6)}
      </div>`;
    }).join('');

    card.innerHTML = `<div class="bg-gradient-to-br from-green-900 via-green-800 to-green-700 px-4 py-5 sm:px-6 sm:py-6">
      <div style="display:flex;align-items:center;justify-content:space-between;gap:8px;margin-bottom:12px">
        <div class="inline-flex items-center gap-2 bg-gold-500/20 text-gold-200 border border-gold-400/40 px-3 py-1 rounded-full text-xs sm:text-sm font-semibold">📊 إحصائيات تقدمك</div>
        <span style="color:#f3d77a;font-weight:800;font-size:14px">${pct({ d: allDone, t: all })}% من الدروس</span>
      </div>
      <p class="mb-3 text-sm text-green-100/85 sm:text-base">${s.units.d} وحدة مكتملة من ${s.units.t}</p>
      <div style="margin-bottom:16px">${bar(pct(s.units), 10)}</div>
      <div style="display:grid;grid-template-columns:repeat(auto-fit,minmax(160px,1fr));gap:10px">${items}</div>
      <div style="display:flex;flex-wrap:wrap;gap:16px;margin-top:16px;padding-top:12px;border-top:1px solid rgba(255,255,255,.15);color:#fff;font-size:13px">
        <label style="display:flex;align-items:center;gap:6px;cursor:pointer"><input type="checkbox" data-opt="hide" ${settings.hide ? 'checked' : ''} style="accent-color:#e0b84a;width:16px;height:16px"> إخفاء محتوى الأدبي</label>
        <label style="display:flex;align-items:center;gap:6px;cursor:${settings.hide ? 'not-allowed' : 'pointer'};opacity:${settings.hide ? 0.6 : 1}"><input type="checkbox" data-opt="exclude" ${settings.hide || settings.exclude ? 'checked' : ''} ${settings.hide ? 'disabled' : ''} style="accent-color:#e0b84a;width:16px;height:16px"> استبعاد الأدبي من الإحصائيات</label>
      </div>
    </div>`;
  }

  function ensureCard() {
    if (document.getElementById('nahj-stats')) return;
    const qa = document.getElementById('teacher-qa-heading');
    const target = qa && qa.closest('section');
    if (!target) return;
    const sec = document.createElement('section');
    sec.id = 'nahj-stats';
    sec.className = 'bg-white rounded-2xl border border-green-200/70 shadow-md shadow-green-900/10 overflow-hidden mb-5 sm:mb-7';
    sec.addEventListener('change', (e) => {
      const opt = e.target.getAttribute && e.target.getAttribute('data-opt');
      if (!opt) return;
      settings[opt] = e.target.checked;
      write(SETTINGS_KEY, settings);
      applyHide();
      render();
    });
    target.parentElement.insertBefore(sec, target);
  }

  function applyHide() {
    getUnits().forEach((u) => {
      unitCard(u).style.display = settings.hide && isLit(unitTitle(u)) ? 'none' : '';
    });
  }

  function refresh() {
    collect();
    ensureCard();
    applyHide();
    render();
  }

  // ---------- حفظ المكان (بمرجع وحدة + مسافة منها) ----------
  function anchorInfo() {
    let best = null;
    getUnits().forEach((u) => {
      if (!isVisible(u)) return;
      const top = unitCard(u).getBoundingClientRect().top;
      if (top <= 120) best = { a: unitTitle(u), off: Math.round(-top) };
    });
    return best;
  }

  function save() {
  if (restoring) return;
  const units = getUnits();
  if (!units.length) return; // مش في صفحة القائمة: متكتبش حاجة
  const open = openUnits();
  if (open.length > 1) return;
  const anchor = anchorInfo();
  write(KEY, {
    open: open.length === 1,
    unit: open.length === 1 ? unitTitle(open[0]) : null,
    a: anchor ? anchor.a : null,
    off: anchor ? anchor.off : 0,
    y: Math.round(window.scrollY),
  });
  if (open.length === 1) write(LAST_KEY, unitTitle(open[0]));
}

  function scrollToSaved(saved) {
    const u = saved.a ? findUnit(saved.a) : null;
    if (u) {
      const cardTop = unitCard(u).getBoundingClientRect().top + window.scrollY;
      window.scrollTo({ top: Math.max(0, cardTop + saved.off), behavior: 'auto' });
    } else if (typeof saved.y === 'number') {
      window.scrollTo({ top: saved.y, behavior: 'auto' });
    }
  }

  // ---------- الزرار العايم ----------
  let fab = null;
  let fabSide = read(FAB_KEY, 'right');

  function placeFab() {
    if (!fab) return;
    fab.style.top = 'auto';
    fab.style.bottom = 'calc(24px + env(safe-area-inset-bottom, 0px))';
    fab.style.left = fabSide === 'left' ? '16px' : 'auto';
    fab.style.right = fabSide === 'right' ? '16px' : 'auto';
  }

  function statsAtView() {
    const card = document.getElementById('nahj-stats');
    if (!card) return true;
    return card.getBoundingClientRect().bottom > 120;
  }

  function updateFab() {
    if (!fab) return;
    const has = getUnits().length > 0;
    fab.style.display = has ? 'flex' : 'none';
    const down = statsAtView();
    fab.textContent = down ? '⬇' : '⬆';
    fab.title = down ? 'انزل لمكان توقفك' : 'اطلع للإحصائيات';
  }

  async function goDown() {
    restoring = true;
    let u = openUnits()[0];
    if (!u) {
      const last = read(LAST_KEY, null);
      u = last ? findUnit(last) : null;
      if (u) {
        u.click();
        await waitFor(() => unitCard(u).querySelector(':scope > div button'));
        await sleep(100);
      }
    }
    if (!u) u = getUnits().find(isVisible);
    if (u) {
      u.style.scrollMarginTop = '140px';
      u.scrollIntoView({ behavior: 'smooth', block: 'start' });
    }
    await sleep(900);
    restoring = false;
    save();
    updateFab();
  }

  function goUp() {
    const card = document.getElementById('nahj-stats');
    if (!card) return window.scrollTo({ top: 0, behavior: 'smooth' });
    card.style.scrollMarginTop = '90px';
    card.scrollIntoView({ behavior: 'smooth', block: 'start' });
  }

  function createFab() {
    if (fab) return;
    fab = document.createElement('button');
    fab.id = 'nahj-fab';
    fab.type = 'button';
    fab.style.cssText =
      'position:fixed;z-index:99999;width:54px;height:54px;border-radius:50%;display:none;align-items:center;justify-content:center;' +
      'font-size:22px;line-height:1;color:#0f2a22;background:linear-gradient(135deg,#f3d77a,#d4a017);border:3px solid #14532d;' +
      'box-shadow:0 8px 20px rgba(0,0,0,.3);cursor:grab;touch-action:none;user-select:none;transition:left .25s,right .25s;';
    document.body.appendChild(fab);
    placeFab();

    let startX = 0, startY = 0, dragging = false, down = false;
    fab.addEventListener('pointerdown', (e) => {
      down = true; dragging = false;
      startX = e.clientX; startY = e.clientY;
      fab.setPointerCapture(e.pointerId);
    });
    fab.addEventListener('pointermove', (e) => {
      if (!down) return;
      if (!dragging && Math.hypot(e.clientX - startX, e.clientY - startY) > 8) {
        dragging = true;
        fab.style.transition = 'none';
        fab.style.cursor = 'grabbing';
      }
      if (dragging) {
        fab.style.left = e.clientX - 27 + 'px';
        fab.style.right = 'auto';
        fab.style.top = e.clientY - 27 + 'px';
        fab.style.bottom = 'auto';
      }
    });
    fab.addEventListener('pointerup', (e) => {
      if (!down) return;
      down = false;
      fab.style.cursor = 'grab';
      if (dragging) {
        fabSide = e.clientX < window.innerWidth / 2 ? 'left' : 'right';
        write(FAB_KEY, fabSide);
        fab.style.transition = 'left .25s,right .25s';
        placeFab();
        return;
      }
      if (statsAtView()) goDown(); else goUp();
    });
  }

  let scrollTimer = null;
  window.addEventListener('scroll', () => {
    updateFab();
    clearTimeout(scrollTimer);
    scrollTimer = setTimeout(save, 150);
  }, { passive: true });

  document.addEventListener('click', (e) => {
    if (!e.isTrusted) return;
    const btn = e.target.closest('button');
    if (!btn || btn.id === 'nahj-fab') return;

    if (btn.querySelector('h3')) {
      if (!isOpen(btn)) {
        setTimeout(() => {
          getUnits().forEach((u) => { if (u !== btn && isOpen(u)) u.click(); });
          setTimeout(() => {
            btn.style.scrollMarginTop = '140px';
            btn.scrollIntoView({ behavior: 'smooth', block: 'start' });
            setTimeout(save, 600);
          }, 100);
        }, 30);
        setTimeout(refresh, 400);
      } else {
        setTimeout(save, 200);
      }
      return;
    }

    if (btn.querySelector('h4')) save();
  }, true);

  window.addEventListener('pagehide', save);

  // ---------- استرجاع المكان ----------
  async function restore(units) {
  restoring = true;
  try {
    const saved = read(KEY, null);
    log('Saved state:', saved);

    let keep = saved && saved.open ? units.find((u) => unitTitle(u) === saved.unit) : null;
    if (keep && settings.hide && isLit(unitTitle(keep))) keep = null;

    let n = 0;
    units.forEach((u) => { if (u !== keep && isOpen(u)) { u.click(); n++; } });
    log(`Units: ${units.length}, collapsed: ${n}`);

    if (keep) {
      if (!isOpen(keep)) keep.click();
      await waitFor(() => openUnits().length === 1 && unitCard(keep).querySelector(':scope > div button'));
    } else {
      await waitFor(() => openUnits().length === 0);
    }

    if (saved) {
      await sleep(200);
      scrollToSaved(saved);
      log('Restored:', saved);
      await sleep(400);
      scrollToSaved(saved); // تثبيت تاني بعد ما الصفحة تستقر
    }
    await sleep(300);
  } finally {
    restoring = false;
    updateFab();
  }
}

  function process() {
    const units = getUnits();
    if (!units.length) return;

    const host = units[0].parentElement.parentElement;
    if (host.dataset.nahjDone) return;
    host.dataset.nahjDone = '1';

    createFab();
    refresh();
    restore(units);
  }

  let timer = null;
  new MutationObserver((muts) => {
    const card = document.getElementById('nahj-stats');
    if (muts.every((m) => (card && card.contains(m.target)) || m.target === fab)) return;
    clearTimeout(timer);
    timer = setTimeout(() => {
      process();
      if (getUnits().length && !document.getElementById('nahj-stats')) refresh();
      updateFab();
    }, 300);
  }).observe(document.documentElement, { childList: true, subtree: true });

  process();
})();