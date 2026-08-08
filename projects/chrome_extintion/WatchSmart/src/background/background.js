// ============================================================
// WatchSmart v2.1 — Background Service Worker
//
// Responsibilities:
//   • Core service worker for the extension.
// ============================================================

chrome.runtime.onInstalled.addListener(() => {
    console.log('WatchSmart: Extension installed/updated.');
});
