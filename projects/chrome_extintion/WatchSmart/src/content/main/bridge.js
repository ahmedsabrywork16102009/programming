// bridge.js — Listen for speed updates & video commands from the ISOLATED world via postMessage.

import { state }          from './state.js';
import { enforceOnMedia } from './tracker.js';
import { showHUD }        from './hud.js';

function handleSpeedSync(data) {
    const newSpeed = parseFloat(data.speed) || 1.0;
    const isManual = !!data.isManual;

    if (Math.abs(newSpeed - state.desiredSpeed) > 0.001) {
        if (isManual && state.settings.showHud !== false) showHUD(newSpeed);
        state.desiredSpeed = newSpeed;
        state.trackedMedia.forEach(enforceOnMedia);
    }
}

function handleVideoCommand(data) {
    const { command, value } = data;

    // Prefer tracked media; fall back to all media on the page.
    let mediaList = [...state.trackedMedia];
    if (mediaList.length === 0) {
        mediaList = [...document.querySelectorAll('video, audio')];
    }

    for (const m of mediaList) {
        try {
            if (command === 'seek' && typeof value === 'number') {
                const dur = isNaN(m.duration) ? Infinity : m.duration;
                m.currentTime = Math.max(0, Math.min(dur, m.currentTime + value));
            } else if (command === 'togglePlayPause') {
                if (m.paused) {
                    m.play().catch(() => {});
                } else {
                    m.pause();
                }
            }
        } catch (e) {
            console.debug('WatchSmart: media command failed', e);
        }
    }
}

export function initBridge() {
    window.addEventListener('message', (event) => {
        const type = event.data?.type;
        if (type === 'WATCHSMART_SYNC') {
            handleSpeedSync(event.data);
        } else if (type === 'WATCHSMART_VIDEO_CMD') {
            handleVideoCommand(event.data);
        } else if (type === 'WATCHSMART_TOGGLE') {
            state.enabled = !!event.data.enabled;
        } else if (type === 'WATCHSMART_SETTINGS') {
            if (event.data.settings) {
                state.settings = { ...state.settings, ...event.data.settings };
            }
        }
    });
}
