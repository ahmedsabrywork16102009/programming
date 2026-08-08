// ============================================================
// CONFIG - All configurable values in one place. No hardcoding.
// ============================================================

const CONFIG = {
  storageKeys: {
    blacklist:    'blacklistWebsites',
    whitelist:    'whitelistWebsites',
    exceptions:   'exceptionsWebsites',
    mode:         'blockingMode',
    migrated:     'migratedToDualLists',
  },

  defaults: {
    blacklist:  ["facebook.com", "tiktok.com", "instagram.com", "twitter.com", "x.com"],
    whitelist:  ["google.com"],
    exceptions: ["github.com"],
    mode:       'blacklist',
  },

  bypassProtocols: [
    'chrome:', 'chrome-extension:', 'edge:',
    'about:', 'brave:', 'view-source:', 'file:',
  ],

  bypassHostnames: ['localhost', '127.0.0.1'],

  blockedPagePath: 'blocked/blocked.html',
};


// ============================================================
// STORAGE - Read / Write helpers
// ============================================================

function readStorage(keys) {
  return new Promise(resolve => {
    chrome.storage.local.get(keys, resolve);
  });
}

function writeStorage(data) {
  return new Promise(resolve => {
    chrome.storage.local.set(data, resolve);
  });
}


// ============================================================
// INITIALIZATION - First install migration
// ============================================================

function buildDefaultData(existingBlacklist) {
  return {
    [CONFIG.storageKeys.blacklist]:   existingBlacklist || CONFIG.defaults.blacklist,
    [CONFIG.storageKeys.whitelist]:   CONFIG.defaults.whitelist,
    [CONFIG.storageKeys.exceptions]:  CONFIG.defaults.exceptions,
    [CONFIG.storageKeys.mode]:        CONFIG.defaults.mode,
    [CONFIG.storageKeys.migrated]:    true,
  };
}

async function migrateIfNeeded() {
  const result = await readStorage([CONFIG.storageKeys.blacklist, CONFIG.storageKeys.migrated]);

  if (result[CONFIG.storageKeys.migrated]) return;

  const data = buildDefaultData(result[CONFIG.storageKeys.blacklist]);
  await writeStorage(data);
  console.log('✅ FocusBlock: Initialized with default lists.');
}

chrome.runtime.onInstalled.addListener(() => {
  migrateIfNeeded();
});


// ============================================================
// URL PARSING - Pure functions, no side effects
// ============================================================

function extractHostname(url) {
  const urlObj = new URL(url);
  return {
    protocol: urlObj.protocol,
    hostname: urlObj.hostname.replace(/^www\./, '').toLowerCase(),
  };
}

function isBypassedProtocol(protocol) {
  return CONFIG.bypassProtocols.includes(protocol);
}

function isBypassedHostname(hostname) {
  return CONFIG.bypassHostnames.includes(hostname);
}

function shouldBypass(protocol, hostname) {
  return isBypassedProtocol(protocol) || isBypassedHostname(hostname);
}

function domainMatches(hostname, domain) {
  const cleaned = domain.replace(/^www\./, '').toLowerCase().trim();
  if (!cleaned) return false;
  return hostname === cleaned || hostname.endsWith('.' + cleaned);
}

function matchesAnyInList(hostname, list) {
  return list.some(domain => domainMatches(hostname, domain));
}


// ============================================================
// BLOCKING DECISION - Pure logic, no side effects
// ============================================================

function isExcepted(hostname, exceptions) {
  return matchesAnyInList(hostname, exceptions);
}

function shouldBlockInBlacklistMode(hostname, blacklist) {
  return matchesAnyInList(hostname, blacklist);
}

function shouldBlockInWhitelistMode(hostname, whitelist) {
  return !matchesAnyInList(hostname, whitelist);
}

function shouldBlock(hostname, mode, blacklist, whitelist) {
  if (mode === 'whitelist') {
    return shouldBlockInWhitelistMode(hostname, whitelist);
  }
  return shouldBlockInBlacklistMode(hostname, blacklist);
}


// ============================================================
// REDIRECTION - Side effect: changes the tab URL
// ============================================================

function buildBlockedPageUrl(originalUrl) {
  const base = chrome.runtime.getURL(CONFIG.blockedPagePath);
  return base + '?url=' + encodeURIComponent(originalUrl);
}

function redirectTab(tabId, blockedPageUrl) {
  chrome.tabs.update(tabId, { url: blockedPageUrl });
}

function blockTab(tabId, originalUrl) {
  const blockedPageUrl = buildBlockedPageUrl(originalUrl);
  redirectTab(tabId, blockedPageUrl);
}


// ============================================================
// MAIN CHECK - Orchestrator, reads storage and decides
// ============================================================

async function loadBlockingState() {
  const keys = [
    CONFIG.storageKeys.blacklist,
    CONFIG.storageKeys.whitelist,
    CONFIG.storageKeys.exceptions,
    CONFIG.storageKeys.mode,
  ];
  const result = await readStorage(keys);

  return {
    blacklist:  result[CONFIG.storageKeys.blacklist]   || [],
    whitelist:  result[CONFIG.storageKeys.whitelist]    || [],
    exceptions: result[CONFIG.storageKeys.exceptions]  || [],
    mode:       result[CONFIG.storageKeys.mode]        || CONFIG.defaults.mode,
  };
}

async function checkAndBlockTab(tabId, url) {
  try {
    const { protocol, hostname } = extractHostname(url);

    if (shouldBypass(protocol, hostname)) return;

    const state = await loadBlockingState();

    if (isExcepted(hostname, state.exceptions)) return;

    if (shouldBlock(hostname, state.mode, state.blacklist, state.whitelist)) {
      console.log(`🚫 FocusBlock: Blocked ${url} (Mode: ${state.mode})`);
      blockTab(tabId, url);
    }
  } catch (error) {
    console.error('⚠️ FocusBlock URL Parsing Error:', error);
  }
}


// ============================================================
// EVENT LISTENER - Entry point
// ============================================================

chrome.tabs.onUpdated.addListener((tabId, changeInfo, tab) => {
  if (changeInfo.status === 'loading' && tab.url) {
    checkAndBlockTab(tabId, tab.url);
  }
});
