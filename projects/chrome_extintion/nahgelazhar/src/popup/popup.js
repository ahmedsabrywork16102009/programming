const KEY = 'nahj_settings';
const DEF = { inline: true, hide: false, exclude: false, sort: true, dropHidden: true, showEmpty: false };
const inputs = Array.from(document.querySelectorAll('input[data-k]'));

function paint(s) {
  inputs.forEach(i => {
    const k = i.dataset.k;
    i.checked = k === 'exclude' ? !!(s.hide || s.exclude) : !!s[k];
    i.disabled = k === 'exclude' && !!s.hide;
  });
}

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
    cur.manualSubs = { u: {}, l: {} };
    chrome.storage.local.set({ [KEY]: cur });
  });
  sure = false;
  resetBtn.classList.remove('sure');
  resetBtn.textContent = '✅ تم المسح';
  setTimeout(() => { resetBtn.textContent = 'مسح تصنيفاتي اليدوية'; }, 1500);
});