const KEY = 'nahj_settings';
const DEF = { newView: true, showLit: true, statsLit: true, sortLessons: true, statsHidden: false, showEmpty: false };
const inputs = Array.from(document.querySelectorAll('input[data-k]'));

function paint(s) { inputs.forEach(i => { i.checked = !!s[i.dataset.k]; }); }

chrome.storage.local.get(KEY, r => paint(Object.assign({}, DEF, r[KEY] || {})));

inputs.forEach(i => i.addEventListener('change', () => {
  chrome.storage.local.get(KEY, r => {
    const cur = r[KEY] || {};
    cur[i.dataset.k] = i.checked;
    chrome.storage.local.set({ [KEY]: cur });
  });
}));

chrome.storage.onChanged.addListener((c, area) => {
  if (area === 'local' && c[KEY]) paint(Object.assign({}, DEF, c[KEY].newValue || {}));
});

const resetBtn = document.getElementById('resetManual');
let sure = false;
resetBtn.addEventListener('click', () => {
  if (!sure) {
    sure = true;
    resetBtn.classList.add('sure');
    resetBtn.textContent = 'اضغط تاني للتأكيد';
    setTimeout(() => { sure = false; resetBtn.classList.remove('sure'); resetBtn.textContent = 'مسح تصنيفاتي اليدوية'; }, 3000);
    return;
  }
  chrome.storage.local.get(KEY, r => {
    const cur = r[KEY] || {};
    cur.manualL = {}; cur.customSubjects = {}; cur.customLec = {};
    chrome.storage.local.set({ [KEY]: cur });
  });
  sure = false;
  resetBtn.classList.remove('sure');
  resetBtn.textContent = '✅ تم المسح';
  setTimeout(() => { resetBtn.textContent = 'مسح تصنيفاتي اليدوية'; }, 1500);
});