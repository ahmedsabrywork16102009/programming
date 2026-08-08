// toggle.js — Enable/Disable the extension via the header toggle switch.
// Persists state to chrome.storage.local under 'extensionEnabled'.

export function initToggle() {
    const toggle = document.getElementById('extension-toggle');
    if (!toggle) return;

    // Load saved state (default: enabled)
    chrome.storage.local.get(['extensionEnabled'], (res) => {
        const enabled = res.extensionEnabled !== false;  // default true
        toggle.checked = enabled;
        applyVisualState(enabled);
    });

    // Toggle click
    toggle.addEventListener('change', () => {
        const enabled = toggle.checked;
        chrome.storage.local.set({ extensionEnabled: enabled });
        applyVisualState(enabled);
    });
}

function applyVisualState(enabled) {
    document.body.classList.toggle('extension-disabled', !enabled);
}
