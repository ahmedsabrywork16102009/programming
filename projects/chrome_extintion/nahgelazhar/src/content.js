(function () {
  'use strict';

  const SETTINGS_KEY = 'nahj_settings', TREES_KEY = 'nahj_trees', OPEN_KEY = 'nahj_open',
        LAST_KEY = 'nahj_last_lesson', ICON_KEY = 'nahj_icons';
  const COURSE_ID = '4c086b90-30b2-44cc-a5f9-d2f74a4c3448';
  const COURSE_KEYS = ['مواد عربية', 'الثالث الثانوي'];

  const norm = s => (s || '').replace(/\s+/g, ' ').trim();
  const nrm = s => norm(s).replace(/[أإآ]/g, 'ا').replace(/ة/g, 'ه');
  const esc = s => String(s).replace(/[&<>"]/g, c => ({ '&': '&amp;', '<': '&lt;', '>': '&gt;', '"': '&quot;' }[c]));
  const sleep = ms => new Promise(r => setTimeout(r, ms));
  const read = (k, d) => { try { const v = JSON.parse(localStorage.getItem(k)); return v == null ? d : v; } catch (_) { return d; } };
  const write = (k, v) => { try { localStorage.setItem(k, JSON.stringify(v)); } catch (_) { } };
  const hookOn = () => document.documentElement.getAttribute('data-nahj-hook') === '1';

  // ===== الإعدادات =====
  const settings = {
    newView: true, showLit: true, statsLit: true, sortLessons: true, statsHidden: false, showEmpty: false, folded: false,
    hiddenSubs: [], hiddenTypes: [], hideDone: false,
    manualL: {}, customSubjects: {}, customLec: {}
  };
  function migrate(m) {
    if (!('showLit' in m)) m.showLit = !m.hide;
    if (!('statsLit' in m)) m.statsLit = !(m.hide || m.exclude);
    if (!('sortLessons' in m)) m.sortLessons = 'sort' in m ? !!m.sort : true;
    if (!('statsHidden' in m)) m.statsHidden = 'dropHidden' in m ? !m.dropHidden : false;
    if (!('newView' in m)) m.newView = true;
    ['hide', 'exclude', 'sort', 'inline', 'dropHidden', 'manualSubs'].forEach(k => delete m[k]);
  }
  function saveSettings() {
    try { chrome.storage.local.set({ [SETTINGS_KEY]: settings }); } catch (_) { write(SETTINGS_KEY, settings); }
  }
  function loadSettings(cb) {
    const old = read(SETTINGS_KEY, {});
    const fin = stored => {
      const m = Object.assign({}, old, stored || {});
      migrate(m);
      Object.assign(settings, m);
      settings.manualL = settings.manualL || {};
      settings.customSubjects = settings.customSubjects || {};
      settings.customLec = settings.customLec || {};
      saveSettings(); cb();
    };
    try { chrome.storage.local.get(SETTINGS_KEY, r => fin(r && r[SETTINGS_KEY])); } catch (_) { fin(null); }
  }

  // ===== الحالة =====
  const trees = read(TREES_KEY, {});
  const icons = read(ICON_KEY, {});
  const openKeys = new Set(read(OPEN_KEY, []));
  const parentOf = new Map();
  let model = null, modelId = null, edit = null, query = '', lastLesson = read(LAST_KEY, null);
  let ready = false, busy = false, active = false, mountedHost = null, userMoved = false, timer = null, reloadArmed = false;

  // واجهة للملفات التانية (الإحصائيات في المرحلة 3)
  const api = window.__nahj = {
    version: 2, listeners: [], settings,
    saveSettings: () => saveSettings(),
    getModel: () => model,
    subName: id => subName(id),
    allSubjects: () => allSubjects(),
    openLesson: id => openLesson(id),
    onRender(fn) { this.listeners.push(fn); }
  };
  function emit() { api.listeners.forEach(f => { try { f(); } catch (e) { console.error('[Nahj]', e); } }); }

  // ===== بيانات الكورس (من رد الموقع) =====
  const TN = { Video: 'فيديو', File: 'ملف', Quiz: 'اختبار', Assignment: 'علامة' };
  const doneOf = l => l.type === 'Video' ? !!l.isCompleted : l.type === 'File' ? !!l.isFileOpened
    : l.type === 'Quiz' ? !!l.isQuizSubmitted : l.type === 'Assignment' ? !!l.isAssignmentSubmitted : false;
  const compact = d => ({
    id: d.id, name: norm(d.courseName),
    units: (d.units || []).filter(u => u.active !== false).map(u => ({
      id: u.id, name: norm(u.unitName), order: u.order,
      lessons: (u.lessons || []).filter(l => l.active !== false).map(l => ({
        id: l.id, name: norm(l.lessonName), type: l.type, done: doneOf(l)
      }))
    }))
  });
  const pathId = () => { const m = location.pathname.match(/\/course(?:-player)?\/([0-9a-f-]{36})/i); return m ? m[1] : null; };
  const nameOk = n => COURSE_KEYS.every(k => nrm(n || '').includes(nrm(k)));
  function isTarget() {
    const id = pathId();
    if (id === COURSE_ID) return true;
    return !!(id && trees[id] && nameOk(trees[id].name));
  }
  function onTree(json) {
    const d = json && json.data;
    if (!d || !d.id || !Array.isArray(d.units)) return;
    if (d.id !== COURSE_ID && !nameOk(d.courseName)) return;
    trees[d.id] = compact(d);
    write(TREES_KEY, trees);
    try { sessionStorage.removeItem('nahj_rl'); } catch (_) { }
    console.log('[Nahj] course data captured:', d.units.length, 'units');
    if (ready) { rebuild(); tick(); }
  }
  window.addEventListener('message', e => {
    if (e.source !== window || !e.data || e.data.source !== 'nahj-hook') return;
    if (e.data.type === 'tree') onTree(e.data.payload);
    else if (e.data.type === 'refetch-failed') location.reload();
  });

  // ===== المواد والتصنيف (الواضح بس، والباقي بتصنفه إنت) =====
  const SUBJECTS = [
    ['nahw', 'نحو', ['نحو']], ['adab', 'أدب', ['ادب']], ['sarf', 'صرف', ['صرف']],
    ['nosos', 'نصوص', ['نصوص']], ['balagha', 'بلاغة', ['بلاغه']]
  ];
  const isLit = t => nrm(t).includes('ادبي');
  const findSubs = txt => { const n = nrm(txt); return SUBJECTS.filter(x => x[2].some(k => n.includes(k))).map(x => x[0]); };
  const allSubjects = () => SUBJECTS.map(x => ({ id: x[0], name: x[1] }))
    .concat(Object.entries(settings.customSubjects).map(([id, name]) => ({ id, name })));
  const subName = id => (allSubjects().find(s => s.id === id) || {}).name || id;

  function classify(u, n, t) {
    const a = [];
    if (t === 'فيديو') a.push('video', n.includes('حل') || n.includes('واجب') ? 'hw' : 'explain');
    else if (t === 'ملف') { if (n.includes('سبورة')) a.push('board'); else if (n.includes('واجب') || n.includes('حل')) a.push('hw'); }
    else if (t === 'اختبار') a.push(u.includes('شامل') || n.includes('شامل') ? 'comp' : n.includes('قبل') ? 'pre' : n.includes('بعد') ? 'post' : 'other');
    return a;
  }
  function rank(u, n, t) {
    if (t === 'علامة') return 99;
    if (t === 'اختبار') return u.includes('شامل') || n.includes('شامل') ? 8 : n.includes('قبل') ? 0 : 7;
    if (t === 'فيديو') return n.includes('حل') || n.includes('واجب') ? 5 : 1;
    if (t === 'ملف') return n.includes('سبورة') ? 2 : n.includes('واجب') || n.includes('حل') ? 6 : 3;
    return 4;
  }
  const sortLessons = arr => arr.slice().sort((a, b) =>
    (settings.sortLessons ? rank(a.uname, a.name, a.tn) - rank(b.uname, b.name, b.tn) : 0) || a.ord - b.ord);

  const ORD = ['', 'الأولى', 'الثانية', 'الثالثة', 'الرابعة', 'الخامسة', 'السادسة', 'السابعة', 'الثامنة', 'التاسعة', 'العاشرة'];
  const TENS = { 20: 'العشرون', 30: 'الثلاثون', 40: 'الأربعون', 50: 'الخمسون' };
  function ordinalAr(n) {
    if (n <= 10) return ORD[n];
    if (n === 11) return 'الحادية عشرة';
    if (n === 12) return 'الثانية عشرة';
    if (n < 20) return ORD[n - 10] + ' عشرة';
    const t = Math.floor(n / 10) * 10, o = n % 10;
    if (!o) return TENS[t] || ('رقم ' + n);
    return (o === 1 ? 'الحادية' : ORD[o]) + ' و' + (TENS[t] || t);
  }
  const key4 = s => nrm(s).replace(/ى/g, 'ي').replace(/\s+/g, '');
  const sameOrd = (a, b) => key4(a) === key4(b);
  const lectureTeacher = name => { const m = norm(name).match(/المحاضرة\s+(.+?)\s*(?:[-–—]|$)/); return m ? m[1].trim() : null; };
  const isMarker = l => l.type === 'Assignment' && /^انتهت محتويات/.test(l.name);
  const markerTeacher = name => norm(name).replace(/[^\u0600-\u06FF\s]/g, '').replace(/^\s*انتهت محتويات\s*/, '')
    .replace(/^المحاضرة\s*/, '').replace(/\s*في مادة.*$/, '').trim() || null;

  function buildModel(tree) {
    const ginfo = {}, units = [];
    tree.units.forEach((u, ui) => {
      const lit = isLit(u.name), groups = [];
      if (u.lessons.some(isMarker)) {
        let cur = [], g = 0;
        u.lessons.forEach((l, li) => {
          cur.push([l, li]);
          if (isMarker(l)) { groups.push({ base: `u:${u.id}#${g}`, teacher: markerTeacher(l.name), marker: true, tail: false, items: cur }); cur = []; g++; }
        });
        if (cur.length) groups.push({ base: `u:${u.id}#${g}`, teacher: null, marker: true, tail: true, items: cur });
      } else groups.push({ base: `u:${u.id}`, teacher: lectureTeacher(u.name), marker: false, tail: false, items: u.lessons.map((l, li) => [l, li]) });
      groups.forEach(gr => {
        ginfo[gr.base] = {
          teacher: gr.teacher, marker: gr.marker, tail: gr.tail, uname: u.name,
          kind: (gr.marker || gr.teacher) ? 'lecture' : 'exam',
          order: u.order * 10000 + (gr.items[0] ? gr.items[0][1] : 0)
        };
      });
      units.push({ u, ui, lit, groups });
    });

    const lessons = [], byId = new Map();
    units.forEach(({ u, ui, lit, groups }) => {
      const U = lit ? [] : findSubs(u.name);
      groups.forEach(gr => gr.items.forEach(([l, li]) => {
        const tn = TN[l.type] || '';
        const o = {
          id: l.id, name: l.name, type: l.type, tn, done: l.done, idx: li, ui, uid: u.id, uname: u.name,
          uorder: u.order, tags: classify(u.name, l.name, tn), lit, gbase: gr.base, sid: null, base: gr.base,
          manual: false, ord: u.order * 10000 + li
        };
        if (!lit) {
          const man = settings.manualL[l.id];
          if (man && man.s) { o.sid = man.s; o.base = man.k || gr.base; o.manual = true; }
          else if (!gr.tail) {
            const L = findSubs(l.name);
            o.sid = L.length === 1 ? L[0] : (L.length === 0 && U.length === 1 ? U[0] : null);
          }
        }
        lessons.push(o); byId.set(o.id, o);
      }));
    });

    const blocks = new Map(), uncl = new Map(), litG = new Map();
    const push = (map, key, init, o) => { let g = map.get(key); if (!g) { g = Object.assign({ lessons: [] }, init); map.set(key, g); } g.lessons.push(o); };
    lessons.forEach(o => {
      if (o.lit) { push(litG, o.uid, { key: 'lg:' + o.uid, title: o.uname, order: o.uorder }, o); return; }
      if (o.sid) {
        const key = o.sid + '|' + o.base;
        let b = blocks.get(key);
        if (!b) {
          const gi = ginfo[o.base], cl = settings.customLec[o.base];
          b = {
            sid: o.sid, base: o.base, key, custom: !!cl, kind: cl ? 'lecture' : (gi ? gi.kind : 'lecture'),
            teacher: gi ? gi.teacher : null, uname: cl ? cl.name : (gi ? gi.uname : o.base),
            order: gi ? gi.order : o.ord, lessons: [], num: 0
          };
          blocks.set(key, b);
        }
        if (b.custom) b.order = Math.min(b.order, o.ord);
        b.lessons.push(o);
        return;
      }
      const gi = ginfo[o.gbase] || {};
      const title = (gi.uname || o.uname) + (gi.marker ? (gi.teacher ? ' · المحاضرة ' + gi.teacher : ' · بعد آخر علامة') : '');
      push(uncl, o.gbase, { base: o.gbase, title, order: gi.order || o.ord }, o);
    });

    const bySub = new Map();
    blocks.forEach(b => { if (!bySub.has(b.sid)) bySub.set(b.sid, []); bySub.get(b.sid).push(b); });
    const subjects = [];
    bySub.forEach((arr, sid) => {
      arr.sort((a, b) => a.order - b.order);
      let n = 0;
      arr.forEach(b => {
        if (b.custom) b.label = b.uname;
        else if (b.kind === 'lecture') {
          n++; b.num = n;
          const o = ordinalAr(n);
          b.label = o + (b.teacher && !sameOrd(o, b.teacher) ? ' · عند المستر: ' + b.teacher : '');
        } else b.label = b.uname;
      });
      subjects.push({ id: sid, name: subName(sid), blocks: arr, ord: arr[0].order });
    });
    subjects.sort((a, b) => a.ord - b.ord);
    return {
      lessons, byId, subjects, ginfo, blockByKey: blocks,
      uncl: [...uncl.values()].sort((a, b) => a.order - b.order),
      lit: [...litG.values()].sort((a, b) => a.order - b.order)
    };
  }
  function rebuild() {
    const id = pathId(), t = id ? trees[id] : null;
    model = t ? buildModel(t) : null;
    modelId = id;
  }

  // ===== DOM =====
  const mainEl = () => document.querySelector('main') || document;
  const getUnits = () => Array.from(mainEl().querySelectorAll('button')).filter(b => b.querySelector('h3'));
  const unitTitle = u => norm(u.querySelector('h3').textContent);
  const host = () => { const u = getUnits()[0]; return u ? u.parentElement.parentElement : null; };
  function waitFor(fn, timeout = 1500) {
    return new Promise(resolve => {
      const t = Date.now();
      (function step() {
        const r = fn();
        if (r) return resolve(r);
        if (Date.now() - t > timeout) return resolve(null);
        setTimeout(step, 16);
      })();
    });
  }

  // ===== العرض =====
  const CHEV = '<svg xmlns="http://www.w3.org/2000/svg" width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m6 9 6 6 6-6"/></svg>';
  const TICK = '<svg xmlns="http://www.w3.org/2000/svg" width="12" height="12" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="3" stroke-linecap="round" stroke-linejoin="round"><path d="M20 6 9 17l-5-5"/></svg>';
  const chev = open => `<span class="text-gray-500" style="display:flex;transition:transform .2s;${open ? 'transform:rotate(180deg)' : ''}">${CHEV}</span>`;
  const bar = p => `<div class="bg-green-100" style="height:6px;border-radius:99px;overflow:hidden"><div class="bg-gold-500" style="width:${p}%;height:100%;border-radius:99px"></div></div>`;
  const cnt = ls => { const d = ls.filter(x => x.done).length; return { d, t: ls.length, p: ls.length ? Math.round(d / ls.length * 100) : 0 }; };
  const smallBtn = 'border border-green-200/70 text-green-800" style="font-size:12px;padding:5px 12px;border-radius:10px;cursor:pointer;background:transparent;font-weight:700';

  const tagsHidden = l => l.tags.some(t => settings.hiddenTypes.includes(t));
  const blockLessons = b => sortLessons(b.lessons).filter(l => !tagsHidden(l));
  const blockVisible = b => { const ls = blockLessons(b); return ls.length > 0 && !(settings.hideDone && ls.every(x => x.done)); };
  const matchQ = l => !query || nrm(l.name).includes(nrm(query));
  function visibleFlat() {
    const out = [];
    model.subjects.forEach(s => {
      if (settings.hiddenSubs.includes(s.id)) return;
      s.blocks.filter(blockVisible).forEach(b => blockLessons(b).forEach(l => out.push(l)));
    });
    return out;
  }
  function nextLesson() {
    const flat = visibleFlat().filter(l => l.type !== 'Assignment');
    if (!flat.length) return null;
    const i = flat.findIndex(l => String(l.id) === String(lastLesson));
    for (let k = i + 1; k < flat.length; k++) if (!flat[k].done) return flat[k];
    return flat.find(l => !l.done) || null;
  }

  function lecOptions(sid, ownBase) {
    const sub = model.subjects.find(s => s.id === sid);
    const opts = (sub ? sub.blocks : []).map(b => ({ base: b.base, label: b.label }));
    if (ownBase && !opts.some(o => o.base === ownBase)) {
      const gi = model.ginfo[ownBase];
      opts.push({ base: ownBase, label: 'وحدتها الأصلية: ' + (gi ? (gi.teacher ? 'المحاضرة ' + gi.teacher : gi.uname) : ownBase) });
    }
    return opts;
  }
  function editorHtml(ownBase) {
    const chip = (attr, val, label, on) => `<button type="button" ${attr}="${esc(val)}" class="${on ? 'bg-green-700 text-white' : 'border border-green-200/70 text-green-800'}" style="font-size:12px;padding:3px 11px;border-radius:999px;cursor:pointer;${on ? 'border:0;' : 'background:transparent;'}">${esc(label)}</button>`;
    let h = `<div class="bg-green-50" style="margin:6px 16px 10px;padding:10px 12px;border-radius:12px"><div class="text-gray-500" style="font-size:12px;margin-bottom:6px">١) اختار المادة</div><div style="display:flex;flex-wrap:wrap;gap:6px">${allSubjects().map(s => chip('data-ps', s.id, s.name, edit.sid === s.id)).join('')}${chip('data-ps', '__ns', '＋ مادة جديدة', false)}</div>`;
    if (edit.sid) {
      const o = lecOptions(edit.sid, ownBase);
      h += `<div class="text-gray-500" style="font-size:12px;margin:10px 0 6px">٢) اختار المحاضرة</div><div style="display:flex;flex-wrap:wrap;gap:6px">${o.map(x => chip('data-pk', x.base, x.label, false)).join('')}${chip('data-pk', '__nl', '＋ محاضرة جديدة', false)}</div>`;
    }
    h += `<div style="display:flex;gap:8px;margin-top:10px"><button type="button" data-auto class="border border-green-200/70 text-green-800" style="font-size:12px;padding:3px 11px;border-radius:999px;cursor:pointer;background:transparent">رجّعها تلقائي</button><button type="button" data-cancel class="border border-green-200/70 text-green-800" style="font-size:12px;padding:3px 11px;border-radius:999px;cursor:pointer;background:transparent">إغلاق</button></div></div>`;
    return h;
  }
  function lessonRow(l, noPill) {
    const hl = String(l.id) === String(lastLesson) ? 'outline:2px solid #d4a017;outline-offset:-3px;' : '';
    const ico = icons[l.type] ? `<img src="${esc(icons[l.type])}" alt="" style="width:26px;height:26px;object-fit:contain">`
      : `<span style="font-size:20px">${{ Video: '🎬', File: '📄', Quiz: '📝', Assignment: '🏁' }[l.type] || '📌'}</span>`;
    const badge = l.done ? `<span class="bg-green-700 text-white" style="position:absolute;bottom:-4px;left:-4px;width:18px;height:18px;border-radius:50%;display:flex;align-items:center;justify-content:center">${TICK}</span>` : '';
    const label = l.sid ? subName(l.sid) : 'غير مصنّف';
    const pill = noPill ? '' : `<button type="button" data-edit="l:${l.id}" title="تصنيف" class="border border-green-200/70 ${l.sid ? 'text-green-700' : 'text-gold-700'}" style="flex-shrink:0;font-size:11px;padding:2px 9px;border-radius:999px;cursor:pointer;background:transparent;${l.manual ? 'font-weight:700;' : ''}">${esc(label)}${l.manual ? ' ✎' : ''}</button>`;
    const ed = !noPill && edit && edit.key === 'l:' + l.id ? editorHtml(l.gbase) : '';
    return `<div class="border-b border-green-50" style="${hl}"><div data-open="${l.id}" class="hover:bg-green-50/40" style="display:flex;align-items:center;gap:12px;padding:10px 16px;cursor:pointer">` +
      `<div class="bg-green-50${l.done ? ' ring-2 ring-green-600/30' : ''}" style="position:relative;width:40px;height:40px;border-radius:12px;display:flex;align-items:center;justify-content:center;flex-shrink:0">${ico}${badge}</div>` +
      `<div style="flex:1;min-width:0"><div class="text-green-900" style="font-size:14px;font-weight:700;word-break:break-word">${esc(l.name)}</div><div style="display:flex;gap:6px;margin-top:3px;flex-wrap:wrap"><span class="border border-green-100 bg-green-50 text-green-800" style="font-size:11px;padding:1px 8px;border-radius:999px">${esc(l.tn)}</span>${l.done ? '<span class="text-green-700" style="font-size:11px;font-weight:700">مكتمل</span>' : ''}</div></div>${pill}</div>${ed}</div>`;
  }
  function sectionShell(inner, cls) {
    return `<div class="bg-white rounded-2xl border ${cls || 'border-green-200/70'} shadow-md shadow-green-900/10 overflow-hidden" data-nahj>${inner}</div>`;
  }
  function blockHtml(s, b) {
    let ls = blockLessons(b);
    if (query) ls = ls.filter(matchQ);
    if (!ls.length) return '';
    const k = 'b:' + s.id + '|' + b.base, open = query ? true : openKeys.has(k);
    parentOf.set(k, 's:' + s.id);
    const c = cnt(ls);
    const badge = `<span class="bg-white border border-green-200/70 text-green-800" style="width:28px;height:28px;border-radius:50%;display:flex;align-items:center;justify-content:center;font-size:13px;font-weight:700;flex-shrink:0">${b.num || '📝'}</span>`;
    let h = `<div data-tg data-k="${esc(k)}" data-p="${esc('s:' + s.id)}" class="border-t border-green-100" style="display:flex;align-items:center;gap:10px;padding:10px 16px;cursor:pointer;background:rgba(240,253,244,.5)">${badge}` +
      `<span class="text-green-900" style="flex:1;min-width:0;font-size:14px;font-weight:700">${esc(b.label)}</span>` +
      `<span class="${c.d === c.t ? 'text-green-700' : 'text-gray-600'}" style="font-size:12px;white-space:nowrap">${c.d === c.t ? '✅ ' : ''}${c.d}/${c.t} دروس</span>${chev(open)}</div>`;
    if (open) h += ls.map(l => lessonRow(l)).join('');
    return h;
  }
  function subjectHtml(s) {
    const blocks = s.blocks.filter(blockVisible);
    const body = blocks.map(b => blockHtml(s, b)).join('');
    if (!body) return '';
    const k = 's:' + s.id, open = query ? true : openKeys.has(k);
    parentOf.set(k, 'root');
    const all = blocks.reduce((a, b) => a.concat(blockLessons(b)), []), c = cnt(all);
    const lec = blocks.filter(b => b.kind === 'lecture').length;
    const nx = all.find(l => !l.done && l.type !== 'Assignment');
    let h = `<div data-tg data-k="${esc(k)}" data-p="root" style="display:flex;align-items:center;gap:12px;padding:14px 16px;cursor:pointer">` +
      `<div class="bg-green-700 text-white" style="width:44px;height:44px;border-radius:12px;display:flex;align-items:center;justify-content:center;font-weight:700;font-size:18px;flex-shrink:0">${esc(s.name.slice(0, 1))}</div>` +
      `<div style="flex:1;min-width:0"><div class="text-green-800 font-thmanyah" style="font-size:17px;font-weight:700">${esc(s.name)}</div><div class="text-gray-500" style="font-size:12px;margin-bottom:6px">${c.d} / ${c.t} درس · ${lec} محاضرة</div>${bar(c.p)}</div>${chev(open)}</div>`;
    if (nx && !query) h += `<div data-open="${nx.id}" class="text-gray-600 hover:bg-green-50/40" style="display:flex;align-items:center;gap:6px;padding:0 16px 10px;font-size:12px;cursor:pointer;word-break:break-word">▶️ الدرس الجاي: <b class="text-green-800">${esc(nx.name)}</b></div>`;
    if (open) h += body;
    return sectionShell(h);
  }
  function groupHtml(g) {
    let ls = sortLessons(g.lessons).filter(l => !tagsHidden(l));
    if (query) ls = ls.filter(matchQ);
    if (!ls.length) return '';
    const k = 'g:' + g.base, open = query ? true : openKeys.has(k);
    parentOf.set(k, 'uc');
    let h = `<div data-tg data-k="${esc(k)}" data-p="uc" class="border-t border-green-100" style="display:flex;align-items:center;gap:8px;padding:10px 16px;cursor:pointer;background:rgba(254,252,232,.5)">` +
      `<span class="text-green-900" style="flex:1;min-width:0;font-size:13px;font-weight:700">${esc(g.title)}</span><span class="text-gray-600" style="font-size:12px">${ls.length}</span>` +
      `<button type="button" data-edit="g:${esc(g.base)}" class="border border-green-200/70 text-green-800" style="font-size:11px;padding:2px 9px;border-radius:999px;cursor:pointer;background:transparent">تصنيف الكل</button>${chev(open)}</div>`;
    if (edit && edit.key === 'g:' + g.base) h += editorHtml(g.base);
    if (open) h += ls.map(l => lessonRow(l)).join('');
    return h;
  }
  function unclassifiedHtml() {
    const body = model.uncl.map(groupHtml).join('');
    if (!body) return '';
    const open = query ? true : openKeys.has('uc');
    parentOf.set('uc', 'root');
    const total = model.uncl.reduce((n, g) => n + g.lessons.length, 0);
    let h = `<div data-tg data-k="uc" data-p="root" style="display:flex;align-items:center;gap:12px;padding:14px 16px;cursor:pointer"><div class="bg-gold-50 text-gold-700 border border-gold-200" style="width:44px;height:44px;border-radius:12px;display:flex;align-items:center;justify-content:center;font-size:20px;flex-shrink:0">🗂️</div><div style="flex:1"><div class="text-gold-700 font-thmanyah" style="font-size:17px;font-weight:700">غير مصنّف</div><div class="text-gray-500" style="font-size:12px">${total} درس مستنيين تصنيفك</div></div>${chev(open)}</div>`;
    if (open) h += body;
    return sectionShell(h, 'border-gold-200');
  }
  function litHtml() {
    if (!settings.showLit || !model.lit.length) return '';
    const all = model.lit.reduce((a, g) => a.concat(g.lessons), []), c = cnt(all);
    const open = query ? true : openKeys.has('lit');
    parentOf.set('lit', 'root');
    let body = model.lit.map(g => {
      const ls = sortLessons(g.lessons).filter(l => !tagsHidden(l) && matchQ(l));
      if (!ls.length) return '';
      const k = g.key, o = query ? true : openKeys.has(k);
      parentOf.set(k, 'lit');
      return `<div data-tg data-k="${esc(k)}" data-p="lit" class="border-t border-green-100" style="display:flex;align-items:center;gap:10px;padding:11px 16px;cursor:pointer;background:rgba(240,253,244,.5)"><span class="text-green-900" style="flex:1;min-width:0;font-size:14px;font-weight:700">${esc(g.title)}</span><span class="text-gray-600" style="font-size:12px">${ls.filter(x => x.done).length}/${ls.length}</span>${chev(o)}</div>` +
        (o ? ls.map(l => lessonRow(l, true)).join('') : '');
    }).join('');
    if (!body) return '';
    let h = `<div data-tg data-k="lit" data-p="root" style="display:flex;align-items:center;gap:12px;padding:14px 16px;cursor:pointer"><div class="bg-green-50 text-green-800 border border-green-200" style="width:44px;height:44px;border-radius:12px;display:flex;align-items:center;justify-content:center;font-size:20px;flex-shrink:0">🎓</div><div style="flex:1"><div class="text-green-800 font-thmanyah" style="font-size:17px;font-weight:700">القسم الأدبي</div><div class="text-gray-500" style="font-size:12px;margin-bottom:6px">${c.d}/${c.t} درس</div>${bar(c.p)}</div>${chev(open)}</div>`;
    if (open) h += body;
    return sectionShell(h);
  }
  function bannerHtml() {
    const msg = !hookOn()
      ? 'الجزء اللي بيلقط بيانات الكورس مش شغال. اتأكد إن Chrome إصدار 111 أو أحدث، وإنك عملت Reload للإضافة من chrome://extensions، وبعدها اضغط F5.'
      : 'مستنيين بيانات الكورس من الموقع… لو ما ظهرتش خلال ثواني اضغط الزرار.';
    return sectionShell(`<div class="text-green-900" style="padding:16px;font-size:14px;line-height:1.8">${msg}<div style="margin-top:10px"><button type="button" data-reload class="${smallBtn}">إعادة تحميل الصفحة</button></div></div>`);
  }
  function contHtml() {
    if (query) return '';
    const l = nextLesson();
    if (!l) return '';
    const b = model.blockByKey.get(l.sid + '|' + l.base);
    return `<div data-open="${l.id}" class="bg-gold-50 border border-gold-200" style="display:flex;align-items:center;gap:10px;padding:10px 14px;border-radius:12px;cursor:pointer"><span style="font-size:20px">▶️</span><div style="flex:1;min-width:0"><div class="text-gold-700" style="font-size:14px;font-weight:700">كمّل من حيث وقفت</div><div class="text-green-900" style="font-size:12px;word-break:break-word">${esc(subName(l.sid))}${b ? ' · ' + esc(b.label) : ''}: ${esc(l.name)}</div></div></div>`;
  }
  function listHtml() {
    if (!model) return bannerHtml();
    const out = model.subjects.filter(s => !settings.hiddenSubs.includes(s.id)).map(subjectHtml).join('') + unclassifiedHtml() + litHtml();
    return out || sectionShell('<div class="text-gray-500" style="padding:16px;font-size:14px">' + (query ? 'مفيش درس بالاسم ده.' : 'مفيش حاجة تتعرض.') + '</div>');
  }
  const $ = id => document.getElementById(id);
  function renderList() { const el = $('nahj-list'); if (!el) return; parentOf.clear(); el.innerHTML = listHtml(); }
  function renderCont() { const el = $('nahj-cont'); if (el) el.innerHTML = model ? contHtml() : ''; }
  function renderView() { renderCont(); renderList(); emit(); }

  // ===== التفاعل =====
  function toggleKey(el, add) {
    const k = el.getAttribute('data-k'), p = el.getAttribute('data-p');
    const sib = [...openKeys].filter(x => x !== k && parentOf.get(x) === p);
    if (openKeys.has(k)) { if (add || !sib.length) openKeys.delete(k); else sib.forEach(x => openKeys.delete(x)); }
    else { if (!add) sib.forEach(x => openKeys.delete(x)); openKeys.add(k); }
    write(OPEN_KEY, [...openKeys]);
    renderList();
  }
  function targetIds() {
    if (edit.key.startsWith('l:')) return [+edit.key.slice(2)];
    const gb = edit.key.slice(2);
    return model.lessons.filter(l => !l.lit && !l.sid && l.gbase === gb).map(l => l.id);
  }
  function finishEdit() { edit = null; saveSettings(); rebuild(); renderView(); }
  function pickSubject(id) {
    if (id === '__ns') {
      const n = prompt('اسم المادة الجديدة؟');
      if (!n || !n.trim()) return;
      id = 'x:' + n.trim();
      settings.customSubjects[id] = n.trim();
      saveSettings();
    }
    edit.sid = id;
    renderList();
  }
  function pickLecture(base) {
    if (!edit || !edit.sid) return;
    if (base === '__nl') {
      const n = prompt('اسم المحاضرة الجديدة؟');
      if (!n || !n.trim()) return;
      base = 'c:' + n.trim();
      settings.customLec[base] = { name: n.trim(), s: edit.sid };
    }
    targetIds().forEach(id => { settings.manualL[id] = { s: edit.sid, k: base }; });
    finishEdit();
  }
  function resetManual() { targetIds().forEach(id => { delete settings.manualL[id]; }); finishEdit(); }
  function onViewClick(e) {
    const t = e.target;
    if (!t.closest) return;
    const ed = t.closest('[data-edit]');
    if (ed) { const k = ed.getAttribute('data-edit'); edit = edit && edit.key === k ? null : { key: k, sid: null }; renderList(); return; }
    const ps = t.closest('[data-ps]'); if (ps) { pickSubject(ps.getAttribute('data-ps')); return; }
    const pk = t.closest('[data-pk]'); if (pk) { pickLecture(pk.getAttribute('data-pk')); return; }
    if (t.closest('[data-auto]')) { resetManual(); return; }
    if (t.closest('[data-cancel]')) { edit = null; renderList(); return; }
    if (t.closest('[data-reload]')) { location.reload(); return; }
    const rf = t.closest('[data-refresh]');
    if (rf) { rf.textContent = 'جاري التحديث…'; window.postMessage({ source: 'nahj-ext', type: 'refetch' }, '*'); setTimeout(() => { rf.textContent = '🔄 تحديث'; }, 4000); return; }
    if (t.closest('[data-collapse]')) { openKeys.clear(); write(OPEN_KEY, []); renderList(); return; }
    const op = t.closest('[data-open]'); if (op) { openLesson(+op.getAttribute('data-open')); return; }
    const tg = t.closest('[data-tg]'); if (tg) toggleKey(tg, e.ctrlKey || e.metaKey);
  }

  // ===== فتح الدرس (من جوا الموقع من غير إعادة تحميل، وإلا بالرابط) =====
  async function openLesson(id) {
    const l = model && model.byId.get(id);
    if (!l || busy) return;
    lastLesson = String(id);
    write(LAST_KEY, lastLesson);
    const url = `/course-player/${pathId()}?unit=${l.ui}&lesson=${l.idx}&lessonId=${l.id}`;
    const btn = getUnits().find(b => unitTitle(b) === l.uname);
    if (!btn) { location.href = url; return; }
    busy = true;
    try {
      btn.click();
      const tabs = await waitFor(() => { const t = document.querySelectorAll('[role="tab"]'); return t.length ? Array.from(t) : null; }, 5000);
      if (!tabs) { location.href = url; return; }
      const title = t => norm(((t.querySelector('span') || {}).textContent) || '');
      let want = tabs[l.idx];
      if (!want || title(want) !== l.name) want = tabs.find(t => title(t) === l.name);
      if (!want) { location.href = url; return; }
      const sel = tabs.find(t => t.getAttribute('aria-selected') === 'true');
      if (sel !== want) want.click();
    } finally { busy = false; }
  }

  // ===== التركيب =====
  function reveal(id) {
    const l = model && model.byId.get(+id);
    if (!l) return;
    if (l.lit) { openKeys.add('lit'); openKeys.add('lg:' + l.uid); }
    else if (l.sid) { openKeys.add('s:' + l.sid); openKeys.add('b:' + l.sid + '|' + l.base); }
    else { openKeys.add('uc'); openKeys.add('g:' + l.gbase); }
  }
  async function scrollToLast() {
    if (!lastLesson) return;
    userMoved = false;
    for (const d of [250, 500]) {
      await sleep(d);
      if (userMoved) return;
      const el = document.querySelector(`[data-open="${lastLesson}"]`);
      if (el) el.scrollIntoView({ block: 'center' });
    }
  }
  ['wheel', 'touchstart', 'keydown'].forEach(ev => window.addEventListener(ev, () => { userMoved = true; }, { passive: true }));

  function ensureShell(h) {
    const v = document.createElement('div');
    v.id = 'nahj-view';
    v.setAttribute('data-nahj', '');
    v.style.cssText = 'display:flex;flex-direction:column;gap:12px;margin-top:8px';
    v.innerHTML = `<div style="display:flex;align-items:center;gap:8px;flex-wrap:wrap;padding:0 4px"><span class="text-green-800 font-thmanyah" style="font-size:18px;font-weight:700;flex:1;min-width:110px">محتوى المواد</span>` +
      `<input id="nahj-q" type="search" placeholder="ابحث عن درس…" class="border border-green-200/70 bg-white text-green-900" style="padding:6px 12px;border-radius:10px;font-size:13px;min-width:150px">` +
      `<button type="button" data-collapse class="${smallBtn}">طي الكل</button><button type="button" data-refresh class="${smallBtn}">🔄 تحديث</button></div>` +
      `<div id="nahj-cont"></div><div id="nahj-list" style="display:flex;flex-direction:column;gap:12px"></div>`;
    v.addEventListener('click', onViewClick);
    v.addEventListener('input', e => { if (e.target.id === 'nahj-q') { query = e.target.value; renderList(); renderCont(); } });
    v.addEventListener('keydown', e => { if (e.key === 'Escape' && e.target.id === 'nahj-q') { e.target.value = ''; query = ''; renderList(); renderCont(); } });
    h.parentNode.insertBefore(v, h.nextSibling);
    return v;
  }
  function mount() {
    const h = host();
    if (!h) return;
    let v = $('nahj-view');
    if (!settings.newView) { h.style.display = ''; if (v) v.remove(); return; }
    if (!v) v = ensureShell(h);
    h.style.display = model ? 'none' : '';
    renderView();
  }
  function cleanup() {
    const v = $('nahj-view');
    if (v) v.remove();
    const h = host();
    if (h) h.style.display = '';
    mountedHost = null;
  }
  function onPlayerTick() {
    const id = new URLSearchParams(location.search).get('lessonId');
    if (id && id !== lastLesson) { lastLesson = id; write(LAST_KEY, id); }
    let dirty = false;
    document.querySelectorAll('[role="tab"] img').forEach(im => {
      const s = im.getAttribute('src') || '';
      const t = /video/i.test(s) ? 'Video' : /pdf/i.test(s) ? 'File' : /quiz/i.test(s) ? 'Quiz' : /homework/i.test(s) ? 'Assignment' : null;
      if (t && icons[t] !== s) { icons[t] = s; dirty = true; }
    });
    if (dirty) write(ICON_KEY, icons);
  }
  function armReload() {
    if (reloadArmed) return;
    reloadArmed = true;
    setTimeout(() => {
      let done = false;
      try { done = !!sessionStorage.getItem('nahj_rl'); } catch (_) { }
      if (!model && !done) { try { sessionStorage.setItem('nahj_rl', '1'); } catch (_) { } location.reload(); }
    }, 4500);
  }
  function tick() {
    if (busy) return;
    if (!isTarget()) { if (active) { cleanup(); active = false; } return; }
    active = true;
    if (location.pathname.includes('/course-player/')) { onPlayerTick(); return; }
    const h = host();
    if (!h) return;
    if (!model || modelId !== pathId()) rebuild();
    if (!model && hookOn()) armReload();
    if (h !== mountedHost) {
      mountedHost = h;
      if (lastLesson && model) reveal(lastLesson);
      mount();
      scrollToLast();
    } else if (!$('nahj-view') || (settings.newView && model && h.style.display !== 'none')) mount();
  }

  function start() {
    loadSettings(() => {
      ready = true;
      try {
        chrome.storage.onChanged.addListener((ch, area) => {
          if (area !== 'local' || !ch[SETTINGS_KEY]) return;
          const nv = ch[SETTINGS_KEY].newValue || {};
          if (Object.keys(nv).every(k => JSON.stringify(nv[k]) === JSON.stringify(settings[k]))) return;
          Object.assign(settings, nv);
          rebuild(); mount();
        });
      } catch (_) { }
      new MutationObserver(m => {
        if (m.every(x => x.target.nodeType === 1 && x.target.closest && x.target.closest('[data-nahj]'))) return;
        clearTimeout(timer);
        timer = setTimeout(tick, 200);
      }).observe(document.getElementById('root') || document.body, { childList: true, subtree: true });
      rebuild();
      tick();
    });
  }
  if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', start);
  else start();
})();