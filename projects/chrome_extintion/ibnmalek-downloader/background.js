const DB_KEY = 'ibnmalek_db_v1';
const SETTINGS_KEY = 'ibnmalek_settings_v1';
const TASK_KEY = 'ibnmalek_task_v1';
const LOG_KEY = 'ibnmalek_log_v1';
const now = () => new Date().toISOString();
async function get(key, fallback) { const x = await chrome.storage.local.get(key); return x[key] ?? fallback; }
async function set(key, value) { await chrome.storage.local.set({[key]: value}); }
async function log(message, level='info') {
  const rows = await get(LOG_KEY, []);
  rows.unshift({time: now(), message: String(message), level});
  await set(LOG_KEY, rows.slice(0, 150));
}
function hash(value) {
  let h1 = 0xdeadbeef, h2 = 0x41c6ce57;
  for (let i=0, ch; i<value.length; i++) { ch = value.charCodeAt(i); h1 = Math.imul(h1 ^ ch, 2654435761); h2 = Math.imul(h2 ^ ch, 1597334677); }
  h1 = Math.imul(h1 ^ (h1>>>16), 2246822507) ^ Math.imul(h2 ^ (h2>>>13), 3266489909);
  h2 = Math.imul(h2 ^ (h2>>>16), 2246822507) ^ Math.imul(h1 ^ (h1>>>13), 3266489909);
  return (4294967296 * (2097151 & h2) + (h1>>>0)).toString(16);
}
function safeName(s) { return (s || 'درس').replace(/[\\/:*?"<>|]/g, '-').replace(/[. ]+$/g, '').trim().slice(0, 100) || 'درس'; }
function lessonFolder(lesson) { const title = safeName((lesson.title || '').replace(/^\s*\d{1,3}\s+/, '')); return `${lesson.number ? lesson.number + ' - ' : ''}${title || 'درس'}`; }
async function saveCollected(payload, tab) {
  const db = await get(DB_KEY, {lessons:{}, coursePages:{}, meta:{createdAt:now()}});
  const pageKey = payload.pageUrl;
  const lessons = payload.lessons || [];
  let added=0, changed=0, unchanged=0;
  for (const incoming of lessons) {
    const old = db.lessons[incoming.key];
    const signature = hash(JSON.stringify({title:incoming.title,teacher:incoming.teacher,lessonUrl:incoming.lessonUrl,youtubeId:incoming.youtubeId}));
    incoming.fingerprint = signature;
    incoming.lastCheckedAt = now();
    if (!old) { db.lessons[incoming.key] = incoming; added++; }
    else {
      const oldSig = old.fingerprint || hash(JSON.stringify({title:old.title,teacher:old.teacher,lessonUrl:old.lessonUrl,youtubeId:old.youtubeId}));
      if (oldSig === signature) {
        db.lessons[incoming.key] = {...old, lastCheckedAt:now(), sourcePage:incoming.sourcePage}; unchanged++;
      } else {
        db.lessons[incoming.key] = {...old, ...incoming, attachments:old.attachments || [], exercisesHtml:old.exercisesHtml || '', updatedAt:now()}; changed++;
      }
    }
  }
  const sourceUrl = tab?.url || payload.pageUrl;
  if (payload.detail && sourceUrl) {
    const u = new URL(sourceUrl); const id = u.searchParams.get('lesson_id');
    const key = `${u.origin}|${id}`;
    const lesson = db.lessons[key] || {id, key, title:payload.pageTitle || `درس ${id}`, number:'', teacher:'', lessonUrl:sourceUrl, sourcePage:sourceUrl, attachments:[], youtubeId:'', youtubeUrl:''};
    const detail = payload.detail;
    const signature = hash(JSON.stringify({attachments:detail.attachments.map(a=>[a.url,a.label,a.kind]), exercisesHtml:detail.exercisesHtml, videoIds:detail.videoIds}));
    const attachments = Array.isArray(detail.attachments)
      ? detail.attachments
      : [];

    const videoIds = Array.isArray(detail.videoIds)
      ? detail.videoIds
      : [];

    const exercisesHtml = detail.exercisesHtml || '';

    if (lesson.detailFingerprint !== signature) {
      lesson.attachments = attachments;
      lesson.exercisesHtml = exercisesHtml;
      lesson.videoIds = videoIds;
      lesson.detailFingerprint = signature;
      lesson.detailUpdatedAt = now();
      lesson.detailChanged = true;
      changed++;
    } else {
      // حدّث البيانات نفسها حتى لو لم يتغير التوقيع.
      lesson.attachments = attachments;
      lesson.exercisesHtml = exercisesHtml;
      lesson.videoIds = videoIds;
      lesson.detailChanged = false;
      unchanged++;
    }

    const hasContent =
      attachments.length > 0 ||
      videoIds.length > 0 ||
      Boolean(exercisesHtml.trim());

    lesson.scanStatus = hasContent ? 'scanned-with-content' : 'scanned-empty';
    lesson.scanError = '';
    lesson.scannedAt = now();
    lesson.lastCheckedAt = now();

    db.lessons[key] = lesson;
  }
  db.coursePages[pageKey] = {url:pageKey, scannedAt:now(), fingerprint:hash(JSON.stringify(lessons.map(l=>[l.key,l.title,l.youtubeId]))), lessonCount:lessons.length};
  db.meta.updatedAt = now();
  await set(DB_KEY, db);
  await log(`حفظ البيانات: ${added} جديد، ${changed} متغير، ${unchanged} دون تغيير.`);
  return {added, changed, unchanged, total:Object.keys(db.lessons).length};
}
async function updateTask(patch) { const old = await get(TASK_KEY, {}); await set(TASK_KEY, {...old,...patch,updatedAt:now()}); }
async function sendToTab(tabId, message) {
  try { return await chrome.tabs.sendMessage(tabId, message); }
  catch { await chrome.scripting.executeScript({target:{tabId}, files:['content.js']}); return await chrome.tabs.sendMessage(tabId, message); }
}
async function downloadAttachments() {
  const db = await get(DB_KEY, {lessons:{}});
  const settings = await get(SETTINGS_KEY, {parallel:3, quality:'best'});
  const jobs=[];
  for (const lesson of Object.values(db.lessons)) {
    for (const file of (lesson.attachments || [])) {
      if (!file.url || file.url.startsWith('blob:') || file.url.startsWith('data:')) continue;
      jobs.push({lesson,file,folder:lessonFolder(lesson)});
    }
  }
  if (!jobs.length) { await log('مفيش مرفقات مباشرة محفوظة. افتح صفحات الدروس واجمع بياناتها الأول.','warn'); await updateTask({status:'idle',current:'لا توجد مرفقات مكتشفة',total:0,done:0,remaining:0,errors:0}); return; }
  const task={status:'running',kind:'download',total:jobs.length,done:0,errors:0,current:'بدء تنزيل المرفقات',startedAt:now(),completedUrls:[]};
  await set(TASK_KEY,task); await log(`بدء تنزيل ${jobs.length} مرفقًا بعدد متزامن ${settings.parallel || 3}.`);
  const completed = new Set(task.completedUrls);
  let cursor=0;
  const worker=async()=>{
    while (cursor<jobs.length) {
      const job=jobs[cursor++];
      if (completed.has(job.file.url)) continue;
      await updateTask({current:`${job.lesson.title}: ${job.file.label}`,remaining:jobs.length-(await get(TASK_KEY,task)).done});
      try {
        const rawName = safeName(job.file.label || job.file.kind || 'ملف');
        const ext = guessExtension(job.file.url);
        const filename = `${job.folder}/${ext && rawName.toLowerCase().endsWith(ext.toLowerCase()) ? rawName : rawName + ext}`;
        await new Promise((resolve,reject)=>chrome.downloads.download({url:job.file.url,filename, conflictAction:'uniquify', saveAs:false}, id=>{const err=chrome.runtime.lastError; if(err) reject(new Error(err.message)); else if(id==null) reject(new Error('تعذر بدء التنزيل')); else resolve(id);}));
        completed.add(job.file.url);
        const current=await get(TASK_KEY,task);
        await updateTask({done:(current.done||0)+1,completedUrls:[...completed]});
        await log(`بدأ تنزيل: ${job.folder}/${job.file.label}`);
      } catch(e) {
        const current=await get(TASK_KEY,task);
        await updateTask({errors:(current.errors||0)+1,done:(current.done||0)+1});
        await log(`فشل تنزيل ${job.file.label}: ${e.message}`,'error');
      }
    }
  };
  await Promise.all(Array.from({length:Math.max(1,Math.min(4,Number(settings.parallel)||3))},worker));
  const final=await get(TASK_KEY,task);
  await updateTask({status:final.errors?'done-with-errors':'done',current:final.errors?`اكتمل مع ${final.errors} أخطاء`:'اكتمل تنزيل المرفقات',finishedAt:now(),remaining:Math.max(0,jobs.length-(final.done||0))});
  await log(`انتهى التنزيل. اكتمل/عولج ${final.done||0} من ${jobs.length}، والأخطاء ${final.errors||0}.`);
}

function waitTabComplete(tabId, timeoutMs=45000) {
  return new Promise((resolve,reject)=>{
    let settled=false;
    const timer=setTimeout(()=>finish(new Error('انتهت مهلة فتح صفحة الدرس')),timeoutMs);
    function finish(err,tab){ if(settled)return; settled=true; clearTimeout(timer); chrome.tabs.onUpdated.removeListener(listener); err?reject(err):resolve(tab); }
    function listener(id,change,tab){ if(id===tabId && change.status==='complete') finish(null,tab); }
    chrome.tabs.onUpdated.addListener(listener);
    chrome.tabs.get(tabId,tab=>{ if(chrome.runtime.lastError)return finish(new Error(chrome.runtime.lastError.message)); if(tab?.status==='complete') finish(null,tab); });
  });
}
async function scanOneLesson(lesson) {
  let tab;
  try {
    tab=await chrome.tabs.create({url:lesson.lessonUrl,active:false});
    await waitTabComplete(tab.id);
    // Give client-rendered lesson widgets a short chance to mount.
    await new Promise(r=>setTimeout(r,450));
    const payload=await sendToTab(tab.id,{type:'COLLECT_PAGE'});
    if(!payload?.ok) throw new Error(payload?.error||'تعذر قراءة تفاصيل الدرس');
    await saveCollected(payload,{url:lesson.lessonUrl});
    return {ok:true,title:lesson.title};
  } catch (e) {
    const db = await get(DB_KEY, { lessons: {}, coursePages: {}, meta: {} });
    const saved = db.lessons[lesson.key];

    if (saved) {
      saved.scanStatus = 'scan-failed';
      saved.scanError = e.message || String(e);
      saved.lastCheckedAt = now();
      db.lessons[lesson.key] = saved;
      await set(DB_KEY, db);
    }

    await log(`فشل فحص ${lesson.title}: ${e.message}`, 'error');

    return {
      ok: false,
      title: lesson.title,
      error: e.message
    };
  }
  finally { if(tab?.id) try { await chrome.tabs.remove(tab.id); } catch {} }
}
async function batchScanLessons() {
  const db=await get(DB_KEY,{lessons:{}});
  const lessons=Object.values(db.lessons||{}).filter(l=>l.lessonUrl && /lesson_id=/.test(l.lessonUrl));
  if(!lessons.length){await log('لا توجد دروس محفوظة. افتح صفحة المقرر واضغط «جمع / تحديث بيانات الصفحة» أولًا.','warn');return;}
  const settings=await get(SETTINGS_KEY,{parallel:3});
  const limit=Math.max(1,Math.min(3,Number(settings.parallel)||2));
  await set(TASK_KEY,{status:'running',kind:'scan',total:lessons.length,done:0,errors:0,current:'بدء فحص تفاصيل الدروس',startedAt:now()});
  await log(`بدء فحص تفاصيل ${lessons.length} درسًا على دفعات، بحد أقصى ${limit} صفحات متزامنة.`);
  let cursor=0;
  const worker=async()=>{while(cursor<lessons.length){const lesson=lessons[cursor++];await updateTask({current:`فحص تفاصيل: ${lesson.title}`});const result=await scanOneLesson(lesson);const t=await get(TASK_KEY,{});await updateTask({done:(t.done||0)+1,errors:(t.errors||0)+(result.ok?0:1)});}}
  await Promise.all(Array.from({length:Math.min(limit,lessons.length)},worker));
  const t=await get(TASK_KEY,{});await updateTask({status:t.errors?'done-with-errors':'done',current:t.errors?`اكتمل الفحص مع ${t.errors} أخطاء`:'اكتمل فحص تفاصيل كل الدروس',finishedAt:now(),remaining:0});
  const latestDb = await get(DB_KEY, { lessons: {} });
  const savedLessons = Object.values(latestDb.lessons || {});

  const scannedWithContent = savedLessons.filter(
    l => l.scanStatus === 'scanned-with-content'
  ).length;

  const scannedEmpty = savedLessons.filter(
    l => l.scanStatus === 'scanned-empty'
  ).length;

  const scanFailed = savedLessons.filter(
    l => l.scanStatus === 'scan-failed'
  ).length;

  const notScanned = savedLessons.filter(
    l => !l.scanStatus
  ).length;

  await log(
    `نتيجة الفحص: وُجد محتوى في ${scannedWithContent} درسًا، ` +
    `${scannedEmpty} دون محتوى مكتشف، ${scanFailed} فشل فحصها، ` +
    `${notScanned} لم تُفحص أو لا تملك حالة فحص محفوظة.`
  );
}
async function openExercisesPrint() {
  const db=await get(DB_KEY,{lessons:{}});
  const lessons=Object.values(db.lessons||{}).filter(l=>l.exercisesHtml && l.exercisesHtml.trim());
  if(!lessons.length){await log('لم يتم اكتشاف محتوى تمارين محفوظ. افتح صفحات الدروس وشغّل جمع البيانات أولًا.','warn');return;}
  await chrome.storage.local.set({ibnmalek_print_payload:{lessons:lessons.map(l=>({number:l.number,title:l.title,html:l.exercisesHtml}))}});
  await chrome.tabs.create({url:chrome.runtime.getURL('print.html')});
  await log(`تم تجهيز صفحة طباعة للتمارين من ${lessons.length} درسًا. استخدم Ctrl+P ثم «حفظ بتنسيق PDF».`);
}

function guessExtension(url) { try { const p=new URL(url).pathname; const m=p.match(/\.(pdf|mp3|m4a|aac|wav|ogg|mp4|m4v|webm|docx?|xlsx?|pptx?|zip|rar|png|jpe?g)(?:$)/i); return m?'.'+m[1]:''; } catch {return '';} }
async function exportCsv() {
  const db=await get(DB_KEY,{lessons:{}}); const rows=Object.values(db.lessons);
  const q=s=>'"'+String(s??'').replace(/"/g,'""')+'"';
  const csv='\ufeff'+[['رقم الدرس','اسم الدرس','المحاضر','رابط الدرس','رابط يوتيوب','عدد المرفقات','آخر فحص'].map(q).join(','),...rows.map(l=>[l.number,l.title,l.teacher,l.lessonUrl,l.youtubeUrl,(l.attachments||[]).length,l.lastCheckedAt].map(q).join(','))].join('\r\n');
  const url='data:text/csv;charset=utf-8,'+encodeURIComponent(csv);
  chrome.downloads.download({url,filename:'ابن-مالك/بيانات-الدروس.csv',saveAs:true,conflictAction:'uniquify'});
  await log(`تصدير CSV لعدد ${rows.length} درسًا.`);
}
chrome.runtime.onMessage.addListener((msg,sender,sendResponse)=>{
  (async()=>{
    try {
      if(msg.type==='COLLECT_RESULT') { const result=await saveCollected(msg.payload,sender.tab); sendResponse({ok:true,result}); }
      else if(msg.type==='DOWNLOAD_ATTACHMENTS') { downloadAttachments(); sendResponse({ok:true}); }
      else if(msg.type==='BATCH_SCAN') { batchScanLessons(); sendResponse({ok:true}); }
      else if(msg.type==='PRINT_EXERCISES') { await openExercisesPrint(); sendResponse({ok:true}); }
      else if(msg.type==='EXPORT_CSV') { await exportCsv(); sendResponse({ok:true}); }
      else if(msg.type==='GET_STATE') { sendResponse({ok:true,db:await get(DB_KEY,{lessons:{},coursePages:{}}),settings:await get(SETTINGS_KEY,{parallel:3,quality:'best'}),task:await get(TASK_KEY,{status:'idle',done:0,total:0}),logs:await get(LOG_KEY,[])}); }
      else if(msg.type==='SAVE_SETTINGS') { await set(SETTINGS_KEY,msg.settings); sendResponse({ok:true}); }
      else if(msg.type==='CLEAR_LOG') { await set(LOG_KEY,[]); sendResponse({ok:true}); }
      else if(msg.type==='CHECK_SAVED') { await checkSavedCourse(); sendResponse({ok:true}); }
      else sendResponse({ok:false,error:'Unknown action'});
    } catch(e) { await log(`خطأ: ${e.message}`,'error'); sendResponse({ok:false,error:e.message}); }
  })(); return true;
});
async function checkSavedCourse() {
  const db=await get(DB_KEY,{lessons:{},coursePages:{}});
  const pages=Object.keys(db.coursePages||{});
  if(!pages.length) { await log('لا توجد صفحة مقرر محفوظة للفحص. افتح صفحة المقرر واضغط جمع البيانات.','warn'); return; }
  await log(`الفحص الدوري جاهز للبدء: يوجد ${pages.length} عنوان صفحة محفوظ. في هذه النسخة يتم إعادة الفحص من الصفحة المفتوحة فقط لتفادي تجاوز الموقع أو طلب بيانات غير مصرح بها.`);
}