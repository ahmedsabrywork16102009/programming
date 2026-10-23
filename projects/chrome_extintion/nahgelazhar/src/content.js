(function () {
  'use strict';

  const LAST_KEY = 'nahj_last_unit', SETTINGS_KEY = 'nahj_settings', CACHE_KEY = 'nahj_stats_cache',
    FAB_KEY = 'nahj_fab_side', ICON_KEY = 'nahj_icons';
  const log = (...a) => console.log('[Nahj Helper]', ...a);
  const norm = s => (s || '').replace(/\s+/g, ' ').trim();
  const nrm = s => norm(s).replace(/[أإآ]/g, 'ا').replace(/ة/g, 'ه');
  const esc = s => String(s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
  const sleep = ms => new Promise(r => setTimeout(r, ms));
  const read = (k, d) => { try { const v = JSON.parse(localStorage.getItem(k)); return v == null ? d : v; } catch (_) { return d; } };
  const write = (k, v) => { try { localStorage.setItem(k, JSON.stringify(v)); } catch (_) { } };

  if ('scrollRestoration' in history) history.scrollRestoration = 'manual';

  // ===== الإعدادات (محفوظة في chrome.storage عشان نافذة الإضافة تتحكم فيها) =====
  const settings = {
    hide: false, exclude: false, sort: true, inline: true, dropHidden: true,
    showEmpty: false, folded: false,
    hiddenSubs: [], hideDone: false, hiddenTypes: [],
    manualSubs: { u: {}, l: {} }
  };
  function saveSettings() {
    try { chrome.storage.local.set({ [SETTINGS_KEY]: settings }); } catch (_) { write(SETTINGS_KEY, settings); }
  }
  function loadSettings(cb) {
    const old = read(SETTINGS_KEY, {});
    const fin = stored => {
      Object.assign(settings, old, stored || {});
      const m = settings.manualSubs || {};
      settings.manualSubs = { u: m.u || {}, l: m.l || {} };
      saveSettings(); cb();
    };
    try { chrome.storage.local.get(SETTINGS_KEY, r => fin(r && r[SETTINGS_KEY])); } catch (_) { fin(null); }
  }

  const cache = read(CACHE_KEY, {});
  const icons = read(ICON_KEY, {});
  let viewMode = 'other';                 // list | unit | other
  let mountedHost = null, lastBox = null, lastSig = '';
  let busy = false, cancelScan = false, userMoved = false, skipRestoreOnce = false;
  let openSet = null;                     // null = آخر وحدة تلقائياً | Set بأسماء الوحدات المفتوحة
  let statsOpen = new Set();              // الصفوف والمواد المفتوحة في الإحصائيات
  let editing = null;                     // العنصر اللي بتعدّل تصنيفه دلوقتي
  let active = false;                     // الإضافة شغالة على الكورس الحالي؟
  let fab = null, fabSide = read(FAB_KEY, 'right'), timer = null, raf = 0;

  // ===== المواد =====
  // [المعرّف، الاسم، كلمات التعرّف عليها (بعد توحيد الهمزات والتاء المربوطة)]
  const SUBJECTS = [
    ['adab', 'أدب', ['ادب', 'اتساع نطاق التعليم']],
    ['nahw', 'نحو', ['نحو', 'اعراب', 'نداء', 'اساليب', 'نواصب', 'جوازم', 'استغاثه', 'ندبه', 'ترخيم']],
    ['nosos', 'نصوص', ['نصوص']],
    ['sarf', 'صرف', ['صرف', 'اشتقاق']],
    ['balagha', 'بلاغة', ['بلاغه']]
  ];
  const SUBJ_IDS = new Set(SUBJECTS.map(x => x[0]));
  // لو وحدة مش بتتصنف لوحدها، اكتب اسمها بالظبط هنا ومادتها، مثال:
  // 'الامتحان الشامل على ما تم شرحه': ['nahw'],
  const UNIT_OVERRIDES = {
  };
  const isLit = t => nrm(t).includes('ادبي');
  const findSubs = txt => { const n = nrm(txt); return SUBJECTS.filter(x => x[2].some(k => n.includes(k))).map(x => x[0]); };
  const subMemo = new Map();
  function subjectsOf(t) {                         // مواد الوحدة: يدوي، وإلا من اسمها، وإلا من أسماء دروسها
    if (subMemo.has(t)) return subMemo.get(t);
    let r = [];
    const man = settings.manualSubs.u[t];
    if (man) r = man.slice();
    else if (!isLit(t)) {
      r = UNIT_OVERRIDES[t] ? UNIT_OVERRIDES[t].slice() : findSubs(t);
      if (!r.length) {
        const e = cache[t], cnt = {};
        ((e && e.lessons) || []).forEach(l => findSubs(l.n).forEach(s => cnt[s] = (cnt[s] || 0) + 1));
        const top = Object.entries(cnt).sort((a, b) => b[1] - a[1])[0];
        if (top) r = [top[0]];
      }
    }
    r = r.filter(id => SUBJ_IDS.has(id));
    subMemo.set(t, r);
    return r;
  }
  function lessonSubs(t, l) {                      // مادة الدرس: يدوي، وإلا من اسمه، وإلا من وحدته
    const man = settings.manualSubs.l[t + '||' + (l.n || '')];
    if (man) return man.filter(id => SUBJ_IDS.has(id));
    if (isLit(t)) return [];
    const own = findSubs(l.n || '');
    return own.length ? own : subjectsOf(t);
  }

  function lessonSubHidden(t, l) {
    const s = lessonSubs(t, l);
    return s.length > 0 && s.every(x => settings.hiddenSubs.includes(x));
  }
  const tagsHidden = l => l.tags.some(x => settings.hiddenTypes.includes(x));
  const lessonGone = (t, l) => tagsHidden(l) || lessonSubHidden(t, l);

  // ===== DOM =====
  const mainEl = () => document.querySelector('main') || document;
  const q = sel => Array.from(mainEl().querySelectorAll(sel));
  const getUnits = () => q('button').filter(b => b.querySelector('h3'));
  const unitTitle = u => norm(u.querySelector('h3').textContent);
  const unitCard = u => u.parentElement;
  const isVisible = u => unitCard(u).offsetParent !== null;
  const findUnit = t => getUnits().find(u => unitTitle(u) === t && isVisible(u));
  const isDone = b => Array.from(b.querySelectorAll('span')).some(s => norm(s.textContent) === 'مكتمل');

  function unitPage() {
    const sec = mainEl().querySelector('section');
    if (!sec || !sec.textContent.includes('دروس الوحدة')) return null;
    const btns = q('button').filter(b => b.querySelector('h4') && !b.closest('[data-nahj]'));
    const h1 = sec.querySelector('h1');
    if (!btns.length || !h1) return null;
    return { title: norm(h1.textContent), btns };
  }
  function syncMode() { viewMode = unitPage() ? 'unit' : (getUnits().length ? 'list' : 'other'); }

  // ===== تحديد الكورس المستهدف =====
  const COURSE_ID = '4c086b90-30b2-44cc-a5f9-d2f74a4c3448';
  const COURSE_KEYS = ['مواد عربية', 'الثالث الثانوي'];
  function isTargetCourse() {
    if (location.pathname.includes(COURSE_ID)) return true;
    const sec = mainEl().querySelector('section');
    if (!sec) return false;
    const t = nrm(sec.textContent);
    return COURSE_KEYS.every(k => t.includes(nrm(k)));
  }
  function cleanup() {
    const c = document.getElementById('nahj-stats');
    if (c) c.remove();
    closePanels();
    getUnits().forEach(u => {
      const card = unitCard(u);
      card.style.display = ''; card.style.outline = ''; card.style.outlineOffset = '';
    });
    mountedHost = null; lastBox = null; lastSig = '';
    if (fab) fab.style.display = 'none';
  }

  function waitFor(fn, timeout = 1500) {
    return new Promise(resolve => {
      const t = Date.now();
      (function tick() {
        const r = fn();
        if (r) return resolve(r);
        if (Date.now() - t > timeout) return resolve(null);
        setTimeout(tick, 16);
      })();
    });
  }

  // ===== تصنيف الدروس =====
  function classify(u, n, t) {
    const a = [];
    if (t === 'فيديو') a.push('video', n.includes('حل') || n.includes('واجب') ? 'hw' : 'explain');
    else if (t === 'ملف') {
      if (n.includes('سبورة')) a.push('board');
      else if (n.includes('واجب') || n.includes('حل')) a.push('hw');
    } else if (t === 'اختبار') {
      a.push(u.includes('شامل') || n.includes('شامل') ? 'comp' : n.includes('قبل') ? 'pre' : n.includes('بعد') ? 'post' : 'other');
    }
    return a;
  }
  function rank(u, n, t) {
    if (t === 'اختبار') return u.includes('شامل') || n.includes('شامل') ? 8 : n.includes('قبل') ? 0 : 7;
    if (t === 'فيديو') return n.includes('حل') || n.includes('واجب') ? 5 : 1;
    if (t === 'ملف') return n.includes('سبورة') ? 2 : n.includes('واجب') || n.includes('حل') ? 6 : 3;
    return 4;
  }
  function lessonType(b) {
    const a = norm(b.querySelector('img')?.alt);
    return a || ['فيديو', 'ملف', 'اختبار'].find(t => b.textContent.includes(t)) || '';
  }

  // ===== صفحة الوحدة الأصلية: ترتيب + إخفاء =====
  function sortLessons(title, btns) {
    if (btns.length < 2) return;
    const box = btns[0].parentElement, list = btns.filter(b => b.parentElement === box);
    if (list.length < 2) return;
    list.forEach((b, i) => { if (b.dataset.nahjOrig === undefined) b.dataset.nahjOrig = i; });
    const items = list.map(b => ({
      b, i: +b.dataset.nahjOrig,
      r: settings.sort ? rank(title, norm(b.querySelector('h4').textContent), lessonType(b)) : 0
    }));
    items.sort((x, y) => x.r - y.r || x.i - y.i);
    if (items.every((x, i) => x.b === list[i])) return;
    items.forEach(x => box.appendChild(x.b));
  }
  function applyLessonVis(title, btns) {
    btns.forEach(b => {
      const n = norm(b.querySelector('h4').textContent);
      const byType = classify(title, n, lessonType(b)).some(t => settings.hiddenTypes.includes(t));
      const subs = lessonSubs(title, { n });
      const bySub = subs.length > 0 && subs.every(x => settings.hiddenSubs.includes(x));
      b.style.display = (byType || bySub) ? 'none' : '';
    });
  }

  // ===== تخزين البيانات =====
  function storeUnit(p) {
    const t = p.title, e = cache[t] || { lessons: [] };
    let iconsDirty = false;
    e.lessons = p.btns.map((b, idx) => {
      const n = norm(b.querySelector('h4').textContent), ty = lessonType(b);
      const src = b.querySelector('img')?.getAttribute('src');
      if (src && icons[ty] !== src) { icons[ty] = src; iconsDirty = true; }
      return { n, ty, i: b.dataset.nahjOrig !== undefined ? +b.dataset.nahjOrig : idx, tags: classify(t, n, ty), done: isDone(b) };
    });
    e.total = e.lessons.length;
    e.done = e.lessons.filter(l => l.done).length;
    subMemo.clear();
    cache[t] = e;
    write(CACHE_KEY, cache);
    if (iconsDirty) write(ICON_KEY, icons);
  }
  function collectList() {
    let changed = false;
    getUnits().forEach(u => {
      const m = u.textContent.match(/(\d+)\s*\/\s*(\d+)\s*دروس/);
      if (!m) return;
      const t = unitTitle(u), e = cache[t] || (cache[t] = { lessons: [] });
      if (e.done !== +m[1] || e.total !== +m[2]) { e.done = +m[1]; e.total = +m[2]; changed = true; }
    });
    if (changed) write(CACHE_KEY, cache);
  }
  function needScan(t) {
    const e = cache[t];
    if (!e || !e.lessons || !e.lessons.length) return true;
    return e.lessons.length !== e.total || e.lessons.filter(l => l.done).length !== e.done;
  }
  const hasTitles = t => { const e = cache[t]; return !!(e && e.lessons && e.lessons.length && e.lessons.every(l => l.n)); };
  const needData = t => !hasTitles(t) || needScan(t);
  function isComplete(t) { const e = cache[t]; return !!(e && e.total && e.done === e.total); }

  // الوحدة تختفي لو كل موادها مخفية، أو كل دروسها مخفية (بالنوع أو بالمادة)
  function subjectHidden(t) { const s = subjectsOf(t); return s.length > 0 && s.every(x => settings.hiddenSubs.includes(x)); }
  function allLessonsHidden(t) {
    const e = cache[t];
    return !!((settings.hiddenTypes.length || settings.hiddenSubs.length) && e && e.lessons && e.lessons.length && e.lessons.every(l => lessonGone(t, l)));
  }
  const unitDropped = t => subjectHidden(t) || allLessonsHidden(t);
  function unitHidden(u) {
    const t = unitTitle(u);
    if (settings.hide && isLit(t)) return true;
    if (unitDropped(t)) return true;
    if (settings.hideDone && isComplete(t)) return true;
    return false;
  }

  const subLabel = ids => ids.map(id => (SUBJECTS.find(x => x[0] === id) || [])[1]).filter(Boolean).join(' + ') || 'عام';

  // أسماء اللي جوا كل صف في الإحصائيات
  function listFor(key) {
    const items = [];
    getUnits().forEach(u => {
      const t = unitTitle(u);
      if (isLit(t) && (settings.hide || settings.exclude)) return;
      const e = cache[t];
      if (!e) return;
      const full = !!(e.total && e.done === e.total);
      const unitItem = () => ({ n: t, done: full, x: e.done + '/' + e.total, ek: 'u:' + t, sub: subjectsOf(t), man: !!settings.manualSubs.u[t] });
      if (key === 'u') { if (e.total && !(settings.dropHidden && unitDropped(t))) items.push(unitItem()); return; }
      if (key.startsWith('s:')) { if (e.total && subjectsOf(t).includes(key.slice(2))) items.push(unitItem()); return; }
      if (settings.dropHidden && unitDropped(t)) return;
      const k = key.slice(2);
      (e.lessons || []).forEach(l => {
        if (settings.dropHidden && lessonSubHidden(t, l)) return;
        if (!l.tags.includes(k)) return;
        const lk = t + '||' + l.n;
        items.push({
          n: l.n || 'درس (حدّث الدروس لعرض الاسم)', u: t, done: l.done,
          ek: l.n ? 'l:' + lk : '', sub: lessonSubs(t, l), man: !!(l.n && settings.manualSubs.l[lk])
        });
      });
    });
    return items;
  }

  // ===== الدروس جوا الصفحة (الشكل القديم) =====
  const CHEV = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="mt-2 h-4 w-4 shrink-0 text-gray-400 sm:mt-0 sm:h-5 sm:w-5" aria-hidden="true"><path d="m15 18-6-6 6-6"></path></svg>';
  const CHK = '<svg xmlns="http://www.w3.org/2000/svg" width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" class="h-3.5 w-3.5" aria-hidden="true"><circle cx="12" cy="12" r="10"></circle><path d="m9 12 2 2 4-4"></path></svg>';
  const mk = (tag, cls, txt) => { const e = document.createElement(tag); if (cls) e.className = cls; if (txt != null) e.textContent = txt; return e; };

  function rowEl(l) {
    const b = mk('button', 'flex w-full min-h-14 items-start gap-3 border-b border-green-50 p-3.5 text-right last:border-b-0 hover:bg-green-50/40 sm:items-center sm:gap-4 sm:p-4');
    b.type = 'button'; b.setAttribute('data-nahj', ''); b.dataset.nahjLesson = l.i;
    const ic = mk('div', 'relative flex h-11 w-11 shrink-0 items-center justify-center rounded-xl sm:h-12 sm:w-12 bg-green-50' + (l.done ? ' ring-2 ring-green-600/30' : ''));
    if (icons[l.ty]) { const im = mk('img', 'h-7 w-7 object-contain sm:h-8 sm:w-8'); im.src = icons[l.ty]; im.alt = l.ty; ic.appendChild(im); }
    else ic.textContent = l.ty === 'فيديو' ? '🎬' : l.ty === 'ملف' ? '📄' : '📝';
    if (l.done) { const bd = mk('span', 'absolute -bottom-1 -left-1 flex h-5 w-5 items-center justify-center rounded-full bg-green-700 text-white shadow'); bd.innerHTML = CHK; ic.appendChild(bd); }
    const mid = mk('div', 'min-w-0 flex-1');
    const meta = mk('div', 'flex flex-wrap items-center gap-1.5');
    meta.appendChild(mk('span', 'inline-flex items-center rounded-full px-2 py-0.5 text-[10px] font-semibold sm:text-[11px] border border-green-100 bg-green-50 text-green-800', l.ty));
    if (l.done) meta.appendChild(mk('span', 'text-[10px] font-semibold text-green-700 sm:text-[11px]', 'مكتمل'));
    mid.append(mk('h4', 'mb-1 break-words text-sm font-bold leading-snug text-green-900 sm:text-base', l.n), meta);
    b.append(ic, mid);
    b.insertAdjacentHTML('beforeend', CHEV);
    return b;
  }

  function setChev(u, on) {
    const sv = u.querySelector(':scope > svg');
    if (sv) { sv.style.transition = 'transform .2s'; sv.style.transform = on ? 'rotate(-90deg)' : ''; }
  }
  const removePanel = card => { const p = card.querySelector(':scope > [data-nahj-panel]'); if (p) p.remove(); };
  function closePanels() {
    document.querySelectorAll('[data-nahj-panel]').forEach(p => p.remove());
    getUnits().forEach(u => setChev(u, false));
  }
  function showPanel(u) {
    const t = unitTitle(u), e = cache[t], card = unitCard(u);
    removePanel(card);
    if (!e || !hasTitles(t)) { setChev(u, false); return; }
    const rows = e.lessons.slice()
      .sort((a, b) => (settings.sort ? rank(t, a.n, a.ty) - rank(t, b.n, b.ty) : 0) || a.i - b.i)
      .filter(l => !lessonGone(t, l));
    if (!rows.length) { setChev(u, false); return; }
    const panel = mk('div', 'border-t border-green-100');
    panel.setAttribute('data-nahj', ''); panel.setAttribute('data-nahj-panel', ''); panel.dataset.unit = t;
    rows.forEach(l => panel.appendChild(rowEl(l)));
    if (needScan(t)) {
      const f = mk('button', 'w-full p-2.5 text-center text-xs font-semibold text-green-700 hover:bg-green-50/40', '🔄 تحديث حالة الدروس');
      f.type = 'button'; f.setAttribute('data-nahj', ''); f.dataset.nahjRefresh = '1';
      panel.appendChild(f);
    }
    panel.addEventListener('click', ev => {
      const r = ev.target.closest('[data-nahj-lesson]');
      if (r) { const l = e.lessons.find(x => String(x.i) === r.dataset.nahjLesson); if (l) openLesson(t, l); return; }
      if (ev.target.closest('[data-nahj-refresh]')) loadUnit(t, false);
    });
    card.appendChild(panel);
    setChev(u, true);
  }
  function openTitles() {
    if (openSet === null) { const l = read(LAST_KEY, null); return l ? [l] : []; }
    return Array.from(openSet);
  }
  function syncPanels() {
    if (!settings.inline) { closePanels(); return; }
    const want = new Set(openTitles());
    getUnits().forEach(u => {
      const t = unitTitle(u);
      if (want.has(t) && hasTitles(t) && !unitHidden(u)) showPanel(u);
      else { removePanel(unitCard(u)); setChev(u, false); }
    });
  }
  function scrollToUnit(t, smooth) {
    const u = findUnit(t);
    if (!u) return;
    window.scrollTo({ top: Math.max(0, unitCard(u).getBoundingClientRect().top + window.scrollY - 150), behavior: smooth ? 'smooth' : 'auto' });
  }
  function ensureSet() {
    if (openSet === null) openSet = new Set(Array.from(document.querySelectorAll('[data-nahj-panel]')).map(p => p.dataset.unit));
    return openSet;
  }
  function toggleInline(btn, additive) {
    const t = unitTitle(btn), card = unitCard(btn), set = ensureSet();
    const isOpen = !!card.querySelector(':scope > [data-nahj-panel]');
    if (isOpen) {
      if (additive || set.size <= 1) { set.delete(t); removePanel(card); setChev(btn, false); }
      else { set.clear(); set.add(t); syncPanels(); }
      return;
    }
    if (!additive) set.clear();
    set.add(t);
    openInline(t, additive);
  }
  async function openInline(t, additive) {
    if (!hasTitles(t)) { await loadUnit(t, !additive); return; }
    syncPanels();
    if (!additive) setTimeout(() => scrollToUnit(t, true), 60);
  }

  // ===== أتمتة التنقل (من ورا نسخة ثابتة من الصفحة) =====
  function makeOverlay(text, withBar) {
    const d = document.createElement('div');
    d.id = 'nahj-scan';
    d.style.cssText = 'position:fixed;inset:0;z-index:2147483000;background:rgba(15,42,34,.97);color:#fff;display:flex;flex-direction:column;align-items:center;justify-content:center;gap:14px;font:700 18px Cairo,Tahoma,sans-serif;direction:rtl';
    d.innerHTML = '<div data-m></div>' + (withBar
      ? '<div style="width:min(320px,80vw);height:8px;border-radius:99px;background:rgba(255,255,255,.2);overflow:hidden"><div data-b style="height:100%;width:0;background:#e0b84a;transition:width .2s"></div></div><button type="button" data-x style="background:rgba(255,255,255,.15);color:#fff;border:0;border-radius:10px;padding:6px 18px;font-size:14px;cursor:pointer">إلغاء</button>'
      : '');
    d.querySelector('[data-m]').textContent = text;
    document.body.appendChild(d);
    const x = d.querySelector('[data-x]');
    if (x) x.onclick = () => { cancelScan = true; };
    return { el: d, msg: d.querySelector('[data-m]'), bar: d.querySelector('[data-b]') };
  }
  function makeCover(text) {
    const root = document.getElementById('root'), first = root && root.firstElementChild;
    if (!first) return makeOverlay(text, false);
    const w = document.createElement('div');
    w.id = 'nahj-scan';
    w.style.cssText = 'position:fixed;inset:0;z-index:2147483000;overflow:hidden;background:' + getComputedStyle(first).backgroundColor;
    const c = first.cloneNode(true);
    c.removeAttribute('id');
    c.querySelectorAll('[id]').forEach(e => e.removeAttribute('id'));
    c.querySelectorAll('[data-nahj],[data-nahj-panel]').forEach(e => { e.removeAttribute('data-nahj'); e.removeAttribute('data-nahj-panel'); });
    c.style.pointerEvents = 'none';
    w.appendChild(c);
    const pill = document.createElement('div');
    pill.style.cssText = 'position:fixed;bottom:24px;left:50%;transform:translateX(-50%);background:#14532d;color:#fff;padding:6px 16px;border-radius:99px;font:600 13px Cairo,Tahoma,sans-serif;opacity:0;transition:opacity .2s;direction:rtl';
    pill.textContent = '⏳ ' + text;
    w.appendChild(pill);
    document.body.appendChild(w);
    w.scrollTop = window.scrollY;
    setTimeout(() => { pill.style.opacity = 1; }, 300);
    return { el: w, msg: pill };
  }
  async function goBack() {
    const back = q('button').find(b => norm(b.textContent) === 'المنهج');
    if (back) back.click();
    await waitFor(() => getUnits().length > 0 && !unitPage(), 5000);
  }
  async function enterUnit(t) {
    const u = getUnits().find(x => unitTitle(x) === t);
    if (!u) return null;
    u.click();
    return await waitFor(() => { const x = unitPage(); return x && x.title === t ? x : null; }, 5000);
  }
  async function loadUnit(t, scroll) {
    if (busy) return;
    busy = true;
    const y = window.scrollY;
    const o = makeCover('جاري تحميل الدروس…');
    try {
      const p = await enterUnit(t);
      if (p) storeUnit(p);
      await goBack();
    } catch (err) { log('load error', err); }
    finally {
      busy = false; skipRestoreOnce = true;
      syncMode(); mountedHost = null; tick();
      window.scrollTo(0, y);
      setTimeout(() => { o.el.remove(); if (scroll !== false) scrollToUnit(t, true); }, 80);
    }
  }
  async function openLesson(t, l) {
    if (busy) return;
    busy = true;
    const o = makeCover('جاري فتح الدرس…');
    try {
      const p = await enterUnit(t);
      if (p) {
        storeUnit(p);
        const b = p.btns.find(x => norm(x.querySelector('h4').textContent) === l.n)
          || p.btns.find(x => x.dataset.nahjOrig !== undefined && +x.dataset.nahjOrig === l.i);
        if (b) { b.click(); await waitFor(() => !unitPage(), 2500); }
      }
    } catch (err) { log('open lesson error', err); }
    finally { busy = false; syncMode(); setTimeout(() => { o.el.remove(); tick(); }, 60); }
  }
  async function scanAll(btn) {
    if (busy || viewMode !== 'list') return;
    const todo = getUnits().map(unitTitle).filter(needData);
    if (!todo.length) { flash(btn, '✅ كله محدّث'); return; }
    busy = true; cancelScan = false;
    const y = window.scrollY;
    const o = makeOverlay('', true);
    try {
      for (let i = 0; i < todo.length && !cancelScan; i++) {
        o.msg.textContent = `جاري قراءة الوحدات… ${i + 1} / ${todo.length}`;
        o.bar.style.width = Math.round(i / todo.length * 100) + '%';
        const p = await enterUnit(todo[i]);
        if (p) storeUnit(p);
        await goBack();
      }
    } catch (err) { log('scan error', err); }
    finally {
      busy = false; skipRestoreOnce = true;
      syncMode(); mountedHost = null; tick();
      window.scrollTo(0, y);
      o.el.remove();
    }
  }

  // ===== الكارت (الشكل A) =====
  const CARDS = [
    ['units', '📚', 'الوحدات المكتملة'], ['video', '🎬', 'كل الفيديوهات'], ['explain', '🎓', 'فيديوهات الشرح'],
    ['board', '📝', 'ملفات السبورة'], ['hw', '✍️', 'الواجبات وحلولها'], ['pre', '🔎', 'الاختبارات القبلية'],
    ['post', '✅', 'الاختبارات البعدية'], ['comp', '🏆', 'الاختبارات الشاملة'], ['other', '🧩', 'اختبارات أخرى']
  ];
  const IC = {
    eye: '<path d="M2.062 12.348a1 1 0 0 1 0-.696 10.75 10.75 0 0 1 19.876 0 1 1 0 0 1 0 .696 10.75 10.75 0 0 1-19.876 0"/><circle cx="12" cy="12" r="3"/>',
    off: '<path d="M10.733 5.076a10.744 10.744 0 0 1 11.205 6.575 1 1 0 0 1 0 .696 10.747 10.747 0 0 1-1.444 2.49"/><path d="M14.084 14.158a3 3 0 0 1-4.242-4.242"/><path d="M17.479 17.499a10.75 10.75 0 0 1-15.417-5.151 1 1 0 0 1 0-.696 10.75 10.75 0 0 1 4.446-5.143"/><path d="m2 2 20 20"/>',
    down: '<path d="m6 9 6 6 6-6"/>',
    refresh: '<path d="M3 12a9 9 0 0 1 9-9 9.75 9.75 0 0 1 6.74 2.74L21 8"/><path d="M21 3v5h-5"/><path d="M21 12a9 9 0 0 1-9 9 9.75 9.75 0 0 1-6.74-2.74L3 16"/><path d="M8 16H3v5"/>',
    copy: '<rect width="14" height="14" x="8" y="8" rx="2" ry="2"/><path d="M4 16c-1.1 0-2-.9-2-2V4c0-1.1.9-2 2-2h10c1.1 0 2 .9 2 2"/>',
    check: '<path d="M20 6 9 17l-5-5"/>',
    circle: '<circle cx="12" cy="12" r="10"/>'
  };
  const ic = (n, sz) => `<svg xmlns="http://www.w3.org/2000/svg" width="${sz || 16}" height="${sz || 16}" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round" aria-hidden="true" style="flex-shrink:0">${IC[n]}</svg>`;
  const pct = o => o.t ? Math.round(o.d / o.t * 100) : 0;
  const hbar = (p, h = 6) => `<div class="bg-green-100" style="flex:1;height:${h}px;border-radius:99px;overflow:hidden"><div class="bg-gold-500" style="width:${p}%;height:100%;border-radius:99px"></div></div>`;
  const vbar = p => `<div class="bg-green-100" style="width:5px;height:34px;border-radius:3px;display:flex;align-items:flex-end;overflow:hidden;flex-shrink:0"><div class="bg-gold-500" style="width:100%;height:${p}%"></div></div>`;
  const eyeBtn = (id, on) => `<button type="button" data-eye="${id}" title="${on ? 'إخفاء' : 'إظهار'}" class="text-green-700" style="background:none;border:0;cursor:pointer;padding:4px;display:flex;border-radius:8px">${ic(on ? 'eye' : 'off', 16)}</button>`;
  const actBtn = (act, icon, label) => `<button type="button" data-act="${act}" class="border border-green-200/70 text-green-800" style="display:flex;align-items:center;gap:5px;padding:5px 11px;border-radius:10px;font-size:12px;font-weight:700;cursor:pointer;background:transparent">${ic(icon, 14)}${label}</button>`;

  function subEditor(i) {
    const cur = i.sub;
    const chip = (id, label, on) => `<button type="button" data-sub="${id}" data-ekey="${esc(i.ek)}" class="${on ? 'bg-green-700 text-white' : 'border border-green-200/70 text-green-800'}" style="font-size:12px;padding:2px 10px;border-radius:999px;cursor:pointer;${on ? 'border:0;' : 'background:transparent;'}">${label}</button>`;
    return `<div style="display:flex;flex-wrap:wrap;gap:6px;margin:6px 0 4px">` +
      SUBJECTS.map(([id, l]) => chip(id, l, cur.includes(id))).join('') +
      chip('none', 'عام', i.man && !cur.length) +
      chip('auto', 'تلقائي', !i.man) + '</div>';
  }
  function namesBox(key, title) {
    const items = listFor(key);
    const head = title ? `<div class="text-green-800" style="font-size:13px;font-weight:700;margin-bottom:4px">${esc(title)}</div>` : '';
    if (!items.length) return `<div class="bg-green-50 text-gray-500" style="border-radius:10px;padding:10px 12px;margin:2px 0 8px;font-size:13px">${head}لسه ما اتقرتش. اضغط «تحديث الدروس».</div>`;
    const done = items.filter(i => i.done).length;
    const row = i => {
      const pill = i.ek ? `<button type="button" data-edit="${esc(i.ek)}" title="تعديل التصنيف" class="border border-green-200/70 text-green-700" style="flex-shrink:0;font-size:11px;padding:1px 8px;border-radius:999px;cursor:pointer;background:transparent;${i.man ? 'font-weight:700;' : ''}">${esc(subLabel(i.sub))}${i.man ? ' ✎' : ''}</button>` : '';
      const editor = i.ek && editing === i.ek ? subEditor(i) : '';
      return `<div><div style="display:flex;gap:8px;align-items:flex-start;font-size:13px"><span class="${i.done ? 'text-green-700' : 'text-gray-400'}" style="margin-top:2px">${ic(i.done ? 'check' : 'circle', 14)}</span><span class="text-green-900" style="flex:1;min-width:0;word-break:break-word">${esc(i.n)}${i.u ? ` <span class="text-gray-500" style="font-size:12px">· ${esc(i.u)}</span>` : ''}${i.x ? ` <span class="text-gray-500" style="font-size:12px">${i.x}</span>` : ''}</span>${pill}</div>${editor}</div>`;
    };
    return `<div class="bg-green-50" style="border-radius:10px;padding:8px 12px;margin:2px 0 8px">${head}<div class="text-gray-500" style="font-size:12px;margin-bottom:6px">${done} من ${items.length} مكتمل</div><div style="max-height:260px;overflow:auto;display:flex;flex-direction:column;gap:6px">${items.map(row).join('')}</div></div>`;
  }

  function render() {
    const card = document.getElementById('nahj-stats');
    if (!card) return;
    const s = computeStats();
    const partial = s.scanned < s.nUnits, folded = settings.folded;

    const subOn = id => !settings.hiddenSubs.includes(id);
    const chips = SUBJECTS
      .filter(([id]) => s.subs[id].t > 0 || settings.showEmpty || !subOn(id))
      .map((x, i) => ({ x, i }))
      .sort((a, b) => (subOn(b.x[0]) - subOn(a.x[0])) || (a.i - b.i))
      .map(({ x: [id, l] }) => {
        const o = s.subs[id], on = subOn(id), open = statsOpen.has('s:' + id);
        return `<div data-exp="s:${id}" class="border border-green-200/70${open ? ' bg-green-50' : ''}" style="flex:1 0 108px;display:flex;align-items:center;gap:8px;padding:7px 9px;border-radius:12px;cursor:pointer;${on ? '' : 'opacity:.5;border-style:dashed;'}">` +
          vbar(pct(o)) +
          `<div style="flex:1;min-width:0"><div class="text-green-900" style="font-size:13px;font-weight:700">${l}</div><div class="text-gray-500" style="font-size:12px">${o.d}/${o.t}</div></div>` +
          eyeBtn('s:' + id, on) + '</div>';
      }).join('');
    const subBoxes = SUBJECTS.filter(([id]) => statsOpen.has('s:' + id))
      .map(([id, l]) => `<div style="margin-top:8px">${namesBox('s:' + id, l)}</div>`).join('');

    const typeOn = k => k === 'units' ? !settings.hideDone : !settings.hiddenTypes.includes(k);
    const rows = CARDS
      .filter(([k]) => k === 'units' || s[k].t > 0 || settings.showEmpty || settings.hiddenTypes.includes(k))
      .map((c, i) => ({ c, i }))
      .sort((a, b) => (typeOn(b.c[0]) - typeOn(a.c[0])) || (a.i - b.i))
      .map(({ c: [k, icon, l] }) => {
        const o = s[k], key = k === 'units' ? 'u' : 't:' + k;
        const on = typeOn(k);
        const open = statsOpen.has(key);
        return `<div class="border-t border-green-100"><div data-exp="${key}" style="display:flex;align-items:center;gap:10px;padding:9px 2px;cursor:pointer;${on ? '' : 'opacity:.5;'}">` +
          `<span class="text-green-900" style="width:150px;font-size:13px;display:flex;gap:6px;align-items:center"><span>${icon}</span>${l}</span>` +
          hbar(pct(o)) +
          `<span class="text-gray-600" style="min-width:84px;font-size:12px;text-align:left"><b class="text-green-800">${o.d}</b>/${o.t} · ${pct(o)}%</span>` +
          eyeBtn(key, on) +
          `<span class="text-gray-500" style="display:flex;transition:transform .2s;${open ? 'transform:rotate(180deg)' : ''}">${ic('down', 16)}</span></div>` +
          (open ? namesBox(key) : '') + '</div>';
      }).join('');

    const foldBtn = `<button type="button" data-act="fold" title="${folded ? 'فتح الإحصائيات' : 'طي الإحصائيات'}" class="border border-green-200/70 text-green-800" style="display:flex;padding:4px;border-radius:8px;cursor:pointer;background:transparent;transition:transform .2s;${folded ? '' : 'transform:rotate(180deg)'}">${ic('down', 16)}</button>`;

    const header =
      `<div style="display:flex;flex-wrap:wrap;align-items:center;justify-content:space-between;gap:8px;margin-bottom:12px">` +
      `<div style="display:flex;align-items:center;gap:8px">${foldBtn}<span class="text-green-800 font-thmanyah" style="font-size:16px;font-weight:700">إحصائيات تقدمك</span><span class="bg-gold-50 text-gold-700 border border-gold-200" style="font-size:12px;font-weight:700;padding:2px 10px;border-radius:999px">${pct(s.all)}% من الدروس</span></div>` +
      (folded ? '' : `<div style="display:flex;gap:6px">${actBtn('scan', 'refresh', 'تحديث الدروس')}${actBtn('showall', 'eye', 'إظهار الكل')}</div>`) +
      `</div>`;

    const summary =
      `<div style="display:flex;justify-content:space-between;align-items:baseline;font-size:13px;margin-bottom:6px"><span class="text-green-900">${s.units.d} وحدة مكتملة من ${s.units.t}</span><span class="text-gray-500">${pct(s.units)}%</span></div>` +
      hbar(pct(s.units), 8);

    const details = folded ? '' :
      (chips ? `<div class="text-gray-500" style="font-size:12px;font-weight:700;margin:16px 0 8px">المواد</div><div style="display:flex;gap:6px;overflow-x:auto;padding-bottom:2px">${chips}</div>` : '') +
      subBoxes +
      `<div style="display:flex;justify-content:space-between;align-items:center;margin:16px 0 4px"><span class="text-gray-500" style="font-size:12px;font-weight:700">الأنواع</span>${partial ? `<span class="text-gold-700" style="font-size:12px">محسوبة من ${s.scanned} وحدة من ${s.nUnits} — اضغط «تحديث الدروس»</span>` : ''}</div>` +
      rows;

    card.innerHTML =
      `<button type="button" data-act="copy" title="ضغطة = ملخص · Ctrl + ضغطة = تقرير مفصّل" class="bg-white border border-green-200/70 text-green-800" style="position:absolute;top:-16px;left:50%;transform:translateX(-50%);display:flex;align-items:center;gap:6px;border-radius:999px;padding:5px 14px;font-size:13px;font-weight:700;cursor:pointer;white-space:nowrap;box-shadow:0 1px 3px rgba(0,0,0,.08)">${ic('copy', 15)}<span data-copy-label>نسخ الإحصائيات</span></button>` +
      `<div style="padding:22px 18px ${folded ? 14 : 10}px">` + header + summary + details + '</div>';
  }

  function statsText() {
    const s = computeStats();
    const lines = ['📊 إحصائيات تقدمي في نهج الأزهر', `إجمالي الدروس: ${pct(s.all)}%`, ''];
    CARDS.forEach(([k, i, l]) => { const o = s[k]; if (k !== 'other' || o.t) lines.push(`${i} ${l}: ${o.d} / ${o.t} (${pct(o)}%)`); });
    lines.push('', '🗂️ المواد: ' + SUBJECTS.filter(x => s.subs[x[0]].t).map(x => `${x[1]} ${s.subs[x[0]].d}/${s.subs[x[0]].t}`).join(' | '));
    if (settings.hide || settings.exclude) lines.push('', '(من غير الأدبي)');
    return lines.join('\n');
  }

  // التقرير المفصّل: كل مادة ← وحدة ← درس، وبعدها كل نوع ← دروسه
  function fullReport() {
    const s = computeStats();
    const BAR = '━━━━━━━━━━━━━━━━━━━━';
    const ti = ty => ty === 'فيديو' ? '🎬' : ty === 'ملف' ? '📄' : '📝';
    const mark = d => d ? '✅' : '⬜';
    const units = [];
    getUnits().forEach((u, idx) => {
      const title = unitTitle(u);
      if (isLit(title) && (settings.hide || settings.exclude)) return;
      const e = cache[title];
      if (!e || !e.total) return;
      if (settings.dropHidden && unitDropped(title)) return;
      units.push({ title, e, n: idx + 1 });
    });

    const L = [
      '📊 تقرير تقدمي المفصّل — نهج الأزهر',
      '📅 ' + new Date().toLocaleDateString('ar-EG'),
      BAR,
      `✅ إجمالي الدروس: ${s.all.d}/${s.all.t} (${pct(s.all)}%)`,
      `📚 الوحدات المكتملة: ${s.units.d}/${s.units.t} (${pct(s.units)}%)`,
      '', '📈 حسب النوع:'
    ];
    CARDS.forEach(([k, i, l]) => { if (k !== 'other' || s[k].t) L.push(`   ${i} ${l}: ${s[k].d}/${s[k].t} (${pct(s[k])}%)`); });
    const subLines = SUBJECTS.filter(x => s.subs[x[0]].t).map(x => `   🔹 ${x[1]}: ${s.subs[x[0]].d}/${s.subs[x[0]].t} وحدة (${pct(s.subs[x[0]])}%)`);
    if (subLines.length) L.push('', '🗂️ حسب المادة:', ...subLines);
    if (settings.hide || settings.exclude) L.push('', '(من غير القسم الأدبي)');

    // ---- التفاصيل حسب المادة ----
    const groups = new Map();
    const put = (g, u, l) => {
      if (!groups.has(g)) groups.set(g, new Map());
      const m = groups.get(g);
      if (!m.has(u.title)) m.set(u.title, { n: u.n, e: u.e, ls: [] });
      if (l) m.get(u.title).ls.push(l);
    };
    units.forEach(u => {
      const lessons = u.e.lessons || [];
      if (isLit(u.title)) { if (!lessons.length) put('lit', u, null); lessons.forEach(l => put('lit', u, l)); return; }
      if (!lessons.length) { const us = subjectsOf(u.title); (us.length ? us : ['none']).forEach(g => put(g, u, null)); return; }
      lessons.forEach(l => {
        if (settings.dropHidden && lessonSubHidden(u.title, l)) return;
        const ls = lessonSubs(u.title, l);
        (ls.length ? ls : ['none']).forEach(g => put(g, u, l));
      });
    });
    const gname = { lit: '🎓 القسم الأدبي', none: '📌 عام (مش تابع لمادة معينة)' };

    SUBJECTS.forEach(x => { gname[x[0]] = '🔹 ' + x[1]; });
    L.push('', BAR, '🗂️ التفاصيل حسب المادة', BAR);
    [...SUBJECTS.map(x => x[0]), 'lit', 'none'].forEach(g => {
      const m = groups.get(g);
      if (!m || !m.size) return;
      let gd = 0, gt = 0;
      m.forEach(v => v.ls.forEach(l => { gt++; if (l.done) gd++; }));
      L.push('', `${gname[g]} — ${gd}/${gt} درس (${pct({ d: gd, t: gt })}%)`);
      m.forEach((v, title) => {
        if (!v.ls.length) { L.push(`  ⬜ [${v.n}] ${title} — ${v.e.done}/${v.e.total} (الدروس لسه ما اتقرتش)`); return; }
        const ud = v.ls.filter(l => l.done).length;
        L.push(`  ${mark(ud === v.ls.length)} [${v.n}] ${title} — ${ud}/${v.ls.length}`);
        v.ls.slice().sort((a, b) => a.i - b.i).forEach(l => L.push(`      ${mark(l.done)} ${ti(l.ty)} ${l.n || '—'}`));
      });
    });

    // ---- التفاصيل حسب النوع ----
    L.push('', BAR, '📈 التفاصيل حسب النوع', BAR);
    CARDS.forEach(([k, i, l]) => {
      const items = listFor(k === 'units' ? 'u' : 't:' + k);
      if (!items.length) return;
      L.push('', `${i} ${l} — ${s[k].d}/${s[k].t} (${pct(s[k])}%)`);
      items.forEach(it => L.push(`  ${mark(it.done)} ${it.n}${it.u ? '  ←  ' + it.u : ''}${it.x ? '  (' + it.x + ')' : ''}`));
    });
    L.push('', BAR, '— Nahgelazhar helper');
    return L.join('\n');
  }

  async function copyText(t) {
    try { await navigator.clipboard.writeText(t); return true; }
    catch (_) {
      try {
        const a = document.createElement('textarea');
        a.value = t; a.style.cssText = 'position:fixed;opacity:0;top:0;left:0';
        document.body.appendChild(a); a.select();
        const ok = document.execCommand('copy'); a.remove(); return ok;
      } catch (__) { return false; }
    }
  }
  const flash = (b, txt) => { const old = b.innerHTML; b.textContent = txt; setTimeout(() => { b.innerHTML = old; }, 1500); };

  function applyHide() {
    const last = read(LAST_KEY, null);
    getUnits().forEach(u => {
      const c = unitCard(u), t = unitTitle(u);
      c.style.display = unitHidden(u) ? 'none' : '';
      c.style.outline = t === last ? '2px solid #d4a017' : '';
      c.style.outlineOffset = t === last ? '2px' : '';
    });
  }

  function toggleEye(id) {
    if (id === 'u') settings.hideDone = !settings.hideDone;
    else {
      const [kind, key] = id.split(':');
      const arr = kind === 's' ? settings.hiddenSubs : settings.hiddenTypes;
      const i = arr.indexOf(key);
      if (i >= 0) arr.splice(i, 1); else arr.push(key);
    }
    saveSettings();
    applyHide(); syncPanels(); render();
  }
  function setManual(ek, id) {                     // حفظ التصنيف اليدوي
    const store = ek[0] === 'u' ? settings.manualSubs.u : settings.manualSubs.l, key = ek.slice(2);
    if (id === 'auto') delete store[key];
    else if (id === 'none') store[key] = [];
    else {
      const cur = store[key] ? store[key].slice() : [];
      const i = cur.indexOf(id);
      if (i >= 0) cur.splice(i, 1); else cur.push(id);
      store[key] = cur;
    }
    subMemo.clear();
    saveSettings();
    applyHide(); syncPanels(); render();
  }

  function ensureCard() {
    if (document.getElementById('nahj-stats')) return;
    const qa = document.getElementById('teacher-qa-heading'), target = qa && qa.closest('section');
    if (!target) return;
    const sec = document.createElement('section');
    sec.id = 'nahj-stats';
    sec.setAttribute('data-nahj', '');
    sec.className = 'bg-white rounded-2xl border border-green-200/70 shadow-md shadow-green-900/10 mb-5 sm:mb-7';
    sec.style.cssText = 'position:relative;margin-top:24px';

    sec.addEventListener('click', async e => {
      const t = e.target;
      if (!t.closest) return;
      const eye = t.closest('[data-eye]');
      if (eye) { toggleEye(eye.getAttribute('data-eye')); return; }
      const act = t.closest('[data-act]');
      if (act) {
        const a = act.getAttribute('data-act');
        if (a === 'scan') scanAll(act);
        else if (a === 'fold') { settings.folded = !settings.folded; saveSettings(); render(); }
        else if (a === 'showall') {
          settings.hiddenSubs = []; settings.hiddenTypes = []; settings.hideDone = false;
          saveSettings(); applyHide(); syncPanels(); render();
        } else if (a === 'copy') {
          const full = e.ctrlKey || e.metaKey;
          const lab = act.querySelector('[data-copy-label]'), old = lab.textContent;
          const ok = await copyText(full ? fullReport() : statsText());
          lab.textContent = ok ? (full ? '✅ تم نسخ التقرير المفصّل' : '✅ تم النسخ') : '❌ فشل النسخ';
          setTimeout(() => { lab.textContent = old; }, 1800);
        }
        return;
      }
      const ed = t.closest('[data-edit]');
      if (ed) { const k = ed.getAttribute('data-edit'); editing = editing === k ? null : k; render(); return; }
      const sb = t.closest('[data-sub]');
      if (sb) { setManual(sb.getAttribute('data-ekey'), sb.getAttribute('data-sub')); return; }
      const ex = t.closest('[data-exp]');
      if (ex) {                                      // ضغطة عادية: واحد بس، Ctrl: يضيف على المفتوح
        const k = ex.getAttribute('data-exp'), add = e.ctrlKey || e.metaKey;
        if (statsOpen.has(k)) {
          if (add || statsOpen.size <= 1) statsOpen.delete(k);
          else { statsOpen.clear(); statsOpen.add(k); }
        } else {
          if (!add) statsOpen.clear();
          statsOpen.add(k);
        }
        render();
      }
    });

    target.parentElement.insertBefore(sec, target);
  }

  function refresh() { collectList(); ensureCard(); applyHide(); render(); }

  // ===== الرجوع لمكانك =====
  async function restoreScroll() {
    const last = read(LAST_KEY, null);
    if (!last) return;
    userMoved = false;
    await sleep(150); if (!userMoved) scrollToUnit(last, false);
    await sleep(450); if (!userMoved) scrollToUnit(last, false);
  }
  function restoreOrY() {
    if (skipRestoreOnce) { skipRestoreOnce = false; return; }
    restoreScroll();
  }
  ['wheel', 'touchstart', 'keydown'].forEach(ev => window.addEventListener(ev, () => { userMoved = true; }, { passive: true }));

  // ===== الزرار العائم =====
  function placeFab() {
    if (!fab) return;
    fab.style.top = 'auto';
    fab.style.bottom = 'calc(24px + env(safe-area-inset-bottom, 0px))';
    fab.style.left = fabSide === 'left' ? '16px' : 'auto';
    fab.style.right = fabSide === 'right' ? '16px' : 'auto';
  }
  function statsAtView() { const c = document.getElementById('nahj-stats'); return !c || c.getBoundingClientRect().bottom > 120; }
  function updateFab() {
    if (!fab) return;
    fab.style.display = viewMode === 'list' ? 'flex' : 'none';
    if (viewMode !== 'list') return;
    const d = statsAtView();
    fab.textContent = d ? '⬇' : '⬆';
    fab.title = d ? 'انزل لآخر وحدة فتحتها' : 'اطلع للإحصائيات';
  }
  function goDown() {
    const last = read(LAST_KEY, null);
    const u = (last && findUnit(last)) || getUnits().find(isVisible);
    if (u) window.scrollTo({ top: Math.max(0, unitCard(u).getBoundingClientRect().top + window.scrollY - 150), behavior: 'smooth' });
  }
  function goUp() {
    const c = document.getElementById('nahj-stats');
    if (!c) return window.scrollTo({ top: 0, behavior: 'smooth' });
    c.style.scrollMarginTop = '90px';
    c.scrollIntoView({ behavior: 'smooth', block: 'start' });
  }
  function createFab() {
    if (fab) return;
    fab = document.createElement('button');
    fab.id = 'nahj-fab'; fab.type = 'button';
    fab.style.cssText = 'position:fixed;z-index:99999;width:54px;height:54px;border-radius:50%;display:none;align-items:center;justify-content:center;font-size:22px;line-height:1;color:#0f2a22;background:linear-gradient(135deg,#f3d77a,#d4a017);border:3px solid #14532d;box-shadow:0 8px 20px rgba(0,0,0,.3);cursor:grab;touch-action:none;user-select:none;transition:left .25s,right .25s';
    document.body.appendChild(fab);
    placeFab();
    let x = 0, y = 0, drag = false, down = false;
    fab.addEventListener('pointerdown', e => { down = true; drag = false; x = e.clientX; y = e.clientY; fab.setPointerCapture(e.pointerId); });
    fab.addEventListener('pointermove', e => {
      if (!down) return;
      if (!drag && Math.hypot(e.clientX - x, e.clientY - y) > 8) { drag = true; fab.style.transition = 'none'; fab.style.cursor = 'grabbing'; }
      if (drag) { fab.style.left = e.clientX - 27 + 'px'; fab.style.right = 'auto'; fab.style.top = e.clientY - 27 + 'px'; fab.style.bottom = 'auto'; }
    });
    fab.addEventListener('pointerup', e => {
      if (!down) return;
      down = false; fab.style.cursor = 'grab';
      if (drag) {
        fabSide = e.clientX < innerWidth / 2 ? 'left' : 'right';
        write(FAB_KEY, fabSide);
        fab.style.transition = 'left .25s,right .25s';
        placeFab();
        return;
      }
      statsAtView() ? goDown() : goUp();
    });
  }

  window.addEventListener('scroll', () => {
    if (raf) return;
    raf = requestAnimationFrame(() => { raf = 0; updateFab(); });
  }, { passive: true });

  // اعتراض كليك الوحدة: عادي = وحدة واحدة، Ctrl = تضيف على اللي مفتوح
  document.addEventListener('click', e => {
    if (!e.isTrusted || busy || viewMode !== 'list') return;
    const b = e.target.closest && e.target.closest('button');
    if (!b || !b.querySelector('h3')) return;
    write(LAST_KEY, unitTitle(b));
    applyHide();
    if (settings.inline) {
      e.preventDefault();
      e.stopImmediatePropagation();
      toggleInline(b, e.ctrlKey || e.metaKey);
    }
  }, true);

  // ===== المحرك الرئيسي =====
  function mountList(host) {
    mountedHost = host;
    createFab();
    refresh();
    syncPanels();
    restoreOrY();
  }
  function bounce() {
    if (busy || !settings.inline || !unitPage()) return;
    const back = q('button').find(b => norm(b.textContent) === 'المنهج');
    if (back) back.click();
  }

  function tick() {
    if (busy) return;
    if (!isTargetCourse()) {                        // مش الكورس بتاعنا: ما نلمسش أي حاجة
      if (active) { cleanup(); active = false; }
      viewMode = 'other';
      return;
    }
    active = true;
    const prev = viewMode, p = unitPage();
    if (p) {
      viewMode = 'unit';
      sortLessons(p.title, p.btns);
      applyLessonVis(p.title, p.btns);
      const box = p.btns[0].parentElement;
      const sig = p.title + '|' + p.btns.map(b => norm(b.querySelector('h4').textContent) + (isDone(b) ? 1 : 0)).sort().join('~');
      if (box !== lastBox || sig !== lastSig) {
        lastBox = box; lastSig = sig;
        storeUnit(p);
        write(LAST_KEY, p.title);
        if (openSet === null) openSet = new Set([p.title]); else openSet.add(p.title);
      }
      if (prev !== 'unit' && settings.inline) setTimeout(bounce, 120);
      updateFab();
      return;
    }
    const units = getUnits();
    if (units.length) {
      viewMode = 'list';
      const host = units[0].parentElement.parentElement;
      if (host !== mountedHost) mountList(host);
      else if (!document.getElementById('nahj-stats')) refresh();
    } else viewMode = 'other';
    updateFab();
  }

    function onSettingsChanged() {
    subMemo.clear();
    if (viewMode === 'list') { applyHide(); syncPanels(); render(); }
    else if (viewMode === 'unit') tick();
  }

  loadSettings(() => {
    try {
      chrome.storage.onChanged.addListener((ch, area) => {
        if (area !== 'local' || !ch[SETTINGS_KEY]) return;
        const nv = ch[SETTINGS_KEY].newValue || {};
        if (Object.keys(nv).every(k => JSON.stringify(nv[k]) === JSON.stringify(settings[k]))) return;
        Object.assign(settings, nv);
        onSettingsChanged();
      });
    } catch (_) { }
    new MutationObserver(m => {
      if (m.every(x => x.target.nodeType === 1 && x.target.closest('[data-nahj]'))) return;
      clearTimeout(timer);
      timer = setTimeout(tick, 200);
    }).observe(document.getElementById('root') || document.body, { childList: true, subtree: true });
    tick();
  });
})();