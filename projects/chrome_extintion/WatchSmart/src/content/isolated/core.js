// core.js — State management and speed application in isolated world.

export const DEFAULT_SETTINGS = {
    speedStep: 0.05,
    adSpeed: 16.0,
    defaultSpeed: 1.0,
    minSpeed: 0.1,
    seekSmall: 5,
    seekMedium: 10,
    seekLarge: 30,
    seekExtra: 60,
    showHud: true
};

export let settings = { ...DEFAULT_SETTINGS };
export let currentSpeed = 1.0;

export function isContextValid() {
    return typeof chrome !== 'undefined' && !!chrome.runtime && !!chrome.runtime.id;
}

export function setCurrentSpeed(speed) {
    currentSpeed = speed;
}

export function updateSettings(newSettings) {
    if (newSettings && typeof newSettings === 'object') {
        settings = { ...settings, ...newSettings };
    }
}

export function broadcastSpeedToMainWorld(isManual = false) {
    window.postMessage({
        type: 'WATCHSMART_SYNC',
        speed: currentSpeed,
        isManual: !!isManual
    }, '*');
}

export function applyGlobalSpeed(speed) {
    const min = settings.minSpeed || 0.1;
    const max = settings.adSpeed || 16.0;
    const clamped = Math.max(min, Math.min(max, parseFloat(speed)));

    currentSpeed = parseFloat(clamped.toFixed(2));
    broadcastSpeedToMainWorld(true);

    if (isContextValid()) {
        try {
            chrome.storage.local.set({ globalVideoSpeed: currentSpeed });
        } catch (e) {
            console.debug('WatchSmart: Failed to save speed to storage.', e);
        }
    }
}
