document.addEventListener('DOMContentLoaded', () => {
  // 1. Get blocked URL from search parameters
  const urlParam = new URLSearchParams(window.location.search).get('url');
  const blockedUrlElement = document.getElementById('blockedUrl');
  
  if (urlParam) {
    try {
      const parsedUrl = new URL(urlParam);
      blockedUrlElement.textContent = parsedUrl.hostname;
    } catch {
      blockedUrlElement.textContent = urlParam;
    }
  } else {
    blockedUrlElement.textContent = 'website.com';
  }
  
  // 2. Load mode and customize message
  chrome.storage.local.get(['theme', 'blockingMode'], (result) => {
    // Apply current theme
    if (result.theme === 'dark') {
      document.documentElement.classList.add('dark-mode');
    }
    
    const mode = result.blockingMode || 'blacklist';
    const messageText = document.getElementById('messageText');
    const subTitle = document.getElementById('subTitle');
    
    if (mode === 'whitelist') {
      subTitle.textContent = 'Whitelist Mode Active';
      messageText.textContent = 'This website is blocked because it is not in your Whitelist. Only approved websites can be accessed right now.';
    } else {
      subTitle.textContent = 'Blacklist Mode Active';
      messageText.textContent = 'This website is blocked because you added it to your FocusBlock Blacklist. Avoid distractions and stay productive!';
    }
  });

  // 3. Connect back button
  const backBtn = document.getElementById('backBtn');
  if (backBtn) {
    backBtn.addEventListener('click', goBack);
  }
});

function goBack() {
  if (typeof chrome !== 'undefined' && chrome.tabs && chrome.tabs.update) {
    // Navigate the current tab to the new tab page
    chrome.tabs.update({ url: 'chrome://newtab/' });
  } else {
    window.location.href = 'https://www.google.com';
  }
}
