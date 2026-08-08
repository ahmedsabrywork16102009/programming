// Safety Storage Abstraction
const storage = (typeof chrome !== 'undefined' && chrome.storage && chrome.storage.local) ? chrome.storage.local : {
  get: (keys, callback) => {
    const result = {};
    keys.forEach(key => {
      const val = localStorage.getItem(key);
      try {
        result[key] = val ? JSON.parse(val) : undefined;
      } catch {
        result[key] = val;
      }
    });
    callback(result);
  },
  set: (obj, callback) => {
    Object.keys(obj).forEach(key => {
      localStorage.setItem(key, typeof obj[key] === 'object' ? JSON.stringify(obj[key]) : obj[key]);
    });
    if (callback) callback();
  }
};

// DOM Elements
const websiteInput = document.getElementById('websiteInput');
const addBtn = document.getElementById('addBtn');
const websitesList = document.getElementById('websitesList');
const messageBox = document.getElementById('messageBox');
const clearAllBtn = document.getElementById('clearAllBtn');
const themeToggle = document.getElementById('themeToggle');
const html = document.documentElement;

// List Tab Buttons
const manageBlacklistBtn = document.getElementById('manageBlacklistBtn');
const manageWhitelistBtn = document.getElementById('manageWhitelistBtn');
const manageExceptionsBtn = document.getElementById('manageExceptionsBtn');

// State Variables
let blacklistWebsites = [];
let whitelistWebsites = [];
let exceptionsWebsites = [];

let currentMode = 'blacklist';      // active blocking mode
let currentTab = 'blacklist';       // active list tab

// 1. Initialize Extension Settings
storage.get(['theme', 'blacklistWebsites', 'whitelistWebsites', 'exceptionsWebsites', 'blockingMode'], (result) => {
  // Theme Setup
  const theme = result.theme || 'light';
  applyTheme(theme);

  // Mode Setup
  currentMode = result.blockingMode || 'blacklist';
  currentTab = currentMode; // Initially open the tab matching the active mode

  // Load Lists
  blacklistWebsites = result.blacklistWebsites || ["facebook.com", "tiktok.com", "instagram.com", "twitter.com", "x.com"];
  whitelistWebsites = result.whitelistWebsites || ["google.com"];
  exceptionsWebsites = result.exceptionsWebsites || ["github.com"];

  // Update UI and display list
  updateModeAndTabUI();
  displayActiveList();
});

// Helper to get active list reference based on tab selection
function getActiveList() {
  if (currentTab === 'exceptions') return exceptionsWebsites;
  return currentTab === 'whitelist' ? whitelistWebsites : blacklistWebsites;
}

// Helper to save all three lists to storage
function saveLists(callback) {
  storage.set({
    blacklistWebsites: blacklistWebsites,
    whitelistWebsites: whitelistWebsites,
    exceptionsWebsites: exceptionsWebsites
  }, callback);
}

// 2. Theme Management
function applyTheme(theme) {
  if (theme === 'dark') {
    html.classList.add('dark-mode');
    themeToggle.querySelector('.theme-icon').textContent = '☀️';
  } else {
    html.classList.remove('dark-mode');
    themeToggle.querySelector('.theme-icon').textContent = '🌙';
  }
}

themeToggle.addEventListener('click', () => {
  const isDark = html.classList.contains('dark-mode');
  const newTheme = isDark ? 'light' : 'dark';
  storage.set({ theme: newTheme });
  applyTheme(newTheme);
});

// 3. UI Synchronization (Tabs and Themes)
function updateModeAndTabUI() {
  // Update theme colors based on the active blocking mode
  if (currentMode === 'whitelist') {
    document.body.classList.add('whitelist-theme');
  } else {
    document.body.classList.remove('whitelist-theme');
  }

  // Set list tab active styling
  manageBlacklistBtn.classList.toggle('active', currentTab === 'blacklist');
  manageWhitelistBtn.classList.toggle('active', currentTab === 'whitelist');
  manageExceptionsBtn.classList.toggle('active', currentTab === 'exceptions');

  // Dynamically update input placeholders
  if (currentTab === 'exceptions') {
    websiteInput.placeholder = 'Enter website to bypass (e.g., github.com)';
  } else if (currentTab === 'whitelist') {
    websiteInput.placeholder = 'Enter website to allow (e.g., google.com)';
  } else {
    websiteInput.placeholder = 'Enter website to block (e.g., facebook.com)';
  }
}

// 4. Tab click listeners (changes active tab and sets active blocking mode)
manageBlacklistBtn.addEventListener('click', () => {
  currentTab = 'blacklist';
  currentMode = 'blacklist';
  storage.set({ blockingMode: 'blacklist' }, () => {
    updateModeAndTabUI();
    displayActiveList();
    showMessage('Blacklist Mode Active', 'success');
  });
});

manageWhitelistBtn.addEventListener('click', () => {
  currentTab = 'whitelist';
  currentMode = 'whitelist';
  storage.set({ blockingMode: 'whitelist' }, () => {
    updateModeAndTabUI();
    displayActiveList();
    showMessage('Whitelist Mode Active', 'success');
  });
});

manageExceptionsBtn.addEventListener('click', () => {
  currentTab = 'exceptions';
  // Keep the current blockingMode when managing exceptions
  updateModeAndTabUI();
  displayActiveList();
});

// 5. Website List Display
function displayActiveList() {
  const list = getActiveList();
  websitesList.innerHTML = '';

  if (list.length === 0) {
    let listType = 'blocked';
    if (currentTab === 'whitelist') listType = 'allowed';
    if (currentTab === 'exceptions') listType = 'excepted/bypassed';
    
    websitesList.innerHTML = `<p class="empty-message">No ${listType} websites listed. Add one above!</p>`;
    clearAllBtn.classList.add('hidden');
    return;
  }

  clearAllBtn.classList.remove('hidden');

  list.forEach((website) => {
    const item = document.createElement('div');
    item.className = 'website-item';
    item.innerHTML = `
      <span class="website-name">${website}</span>
      <button class="delete-btn" data-website="${website}">Delete</button>
    `;
    
    item.querySelector('.delete-btn').addEventListener('click', () => {
      removeWebsite(website);
    });
    
    websitesList.appendChild(item);
  });
}

// 6. Add / Move / Remove logic
function addWebsite() {
  const website = websiteInput.value.trim();

  if (!website) {
    showMessage('Please enter a website URL', 'error');
    return;
  }

  // Normalize/Clean URL
  let cleaned = website.toLowerCase();
  cleaned = cleaned.replace(/^(https?:\/\/)?(www\.)?/, '');
  cleaned = cleaned.split('/')[0];

  // Validate format
  const domainRegex = /^([a-z0-9]+(-[a-z0-9]+)*\.)+[a-z]{2,}$/i;
  if (!domainRegex.test(cleaned)) {
    showMessage('Please enter a valid domain (e.g. domain.com)', 'error');
    return;
  }

  const currentList = getActiveList();

  // 1. Check if already in the active list
  if (currentList.includes(cleaned)) {
    showMessage('This website is already in this list', 'error');
    return;
  }

  // 2. Check conflicts across other lists (Conflict Prompt)
  let conflictingListName = '';
  let conflictingListRef = null;

  if (currentTab !== 'blacklist' && blacklistWebsites.includes(cleaned)) {
    conflictingListName = 'Blacklist';
    conflictingListRef = blacklistWebsites;
  } else if (currentTab !== 'whitelist' && whitelistWebsites.includes(cleaned)) {
    conflictingListName = 'Whitelist';
    conflictingListRef = whitelistWebsites;
  } else if (currentTab !== 'exceptions' && exceptionsWebsites.includes(cleaned)) {
    conflictingListName = 'Exceptions';
    conflictingListRef = exceptionsWebsites;
  }

  if (conflictingListName) {
    showConflictPrompt(cleaned, conflictingListName, conflictingListRef, currentList);
    return;
  }

  // 3. Normal Add
  currentList.push(cleaned);
  saveLists(() => {
    showMessage('Added successfully!', 'success');
    websiteInput.value = '';
    displayActiveList();
  });
}

// Conflict Prompt - queries user whether to move domain
function showConflictPrompt(website, otherListName, otherList, currentList) {
  websiteInput.blur(); // dismiss input focus

  let currentTabLabel = 'Blacklist';
  if (currentTab === 'whitelist') currentTabLabel = 'Whitelist';
  if (currentTab === 'exceptions') currentTabLabel = 'Exceptions';

  messageBox.innerHTML = `
    <div style="margin-bottom: 6px;">"${website}" is already in your <strong>${otherListName}</strong>. Move it here?</div>
    <div class="action-btns">
      <button class="action-btn confirm" id="confirmMoveBtn">Move Here</button>
      <button class="action-btn cancel" id="cancelMoveBtn">Cancel</button>
    </div>
  `;
  messageBox.className = `message-box error`; // show as alert styling
  messageBox.classList.remove('hidden');

  if (window.msgTimeout) clearTimeout(window.msgTimeout);

  document.getElementById('confirmMoveBtn').onclick = () => {
    // Remove from the other conflicting list
    const index = otherList.indexOf(website);
    if (index > -1) {
      otherList.splice(index, 1);
    }
    
    // Add to current list
    currentList.push(website);
    
    // Save lists
    saveLists(() => {
      showMessage(`Moved "${website}" to ${currentTabLabel}!`, 'success');
      websiteInput.value = '';
      displayActiveList();
    });
  };

  document.getElementById('cancelMoveBtn').onclick = () => {
    messageBox.classList.add('hidden');
  };
}

function removeWebsite(website) {
  const currentList = getActiveList();
  const index = currentList.indexOf(website);
  if (index > -1) {
    currentList.splice(index, 1);
    saveLists(() => {
      showMessage('Removed successfully!', 'success');
      displayActiveList();
    });
  }
}

function clearAll() {
  let currentListName = 'Blacklist';
  if (currentTab === 'whitelist') currentListName = 'Whitelist';
  if (currentTab === 'exceptions') currentListName = 'Exceptions';
  
  if (confirm(`Are you sure you want to clear your entire ${currentListName} list?`)) {
    if (currentTab === 'exceptions') {
      exceptionsWebsites = [];
    } else if (currentTab === 'whitelist') {
      whitelistWebsites = [];
    } else {
      blacklistWebsites = [];
    }
    
    saveLists(() => {
      showMessage('List cleared successfully', 'success');
      displayActiveList();
    });
  }
}

// 7. Message Box Feedback
function showMessage(text, type) {
  messageBox.textContent = text;
  messageBox.className = `message-box ${type}`;
  
  if (window.msgTimeout) clearTimeout(window.msgTimeout);
  window.msgTimeout = setTimeout(() => {
    messageBox.classList.add('hidden');
  }, 2500);
}

// 8. Action Event Listeners
addBtn.addEventListener('click', addWebsite);
websiteInput.addEventListener('keypress', (e) => {
  if (e.key === 'Enter') addWebsite();
});
clearAllBtn.addEventListener('click', clearAll);
