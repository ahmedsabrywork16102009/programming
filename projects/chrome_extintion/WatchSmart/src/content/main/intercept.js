// intercept.js — Intercept dynamic media creation + block adblock-detection events.

import { captureMedia } from './tracker.js';

const _createElement    = Document.prototype.createElement;
const _addEventListener = EventTarget.prototype.addEventListener;

// ── Keywords that indicate an adblock-detection listener ─────────────
// YouTube and ad networks register listeners with these signatures to
// detect blocking extensions and freeze/pause playback in response.
const AD_BLOCK_EVENT_TYPES = new Set([
    'adblock', 'adblocker', 'adBlockDetected', 'yt-adblock',
]);

const AD_BLOCK_KEYWORDS = [
    'adblock', 'adblocker', 'ad_block', 'detectAdblock',
    'blockAdvert', 'adBlockDetect', 'adblockdetect',
    'ytp-ad-overlay', 'enforcement',
];

function isAdblockListener(type, listener) {
    // 1. Check the event type string directly
    if (AD_BLOCK_EVENT_TYPES.has(type)) return true;

    // 2. Inspect the listener source for known adblock-detection patterns
    if (typeof listener === 'function') {
        try {
            const src = Function.prototype.toString.call(listener);
            return AD_BLOCK_KEYWORDS.some(kw => src.includes(kw));
        } catch { /* toString may throw on native functions — ignore */ }
    }
    return false;
}

export function applyIntercepts() {
    // ── Intercept createElement to capture dynamic <video>/<audio> ───
    Document.prototype.createElement = function (tag) {
        const el = _createElement.apply(this, arguments);
        if (typeof tag === 'string') {
            const t = tag.toLowerCase();
            if (t === 'video' || t === 'audio') captureMedia(el);
        }
        return el;
    };

    // ── Intercept addEventListener ───────────────────────────────────
    EventTarget.prototype.addEventListener = function (type, listener, options) {
        // Capture any media element that registers a listener
        if (this instanceof HTMLMediaElement) captureMedia(this);

        // Drop adblock-detection listeners silently (S2)
        if (typeof type === 'string' && isAdblockListener(type, listener)) {
            return; // Swallow — YouTube never knows its detector ran
        }

        try {
            return _addEventListener.apply(this, arguments);
        } catch {
            // Silently swallow Permissions-Policy violations (e.g. 'unload')
        }
    };
}
