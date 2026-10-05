// intercept.js — Intercept dynamic media creation to capture React/Vue elements.

import { captureMedia } from './tracker.js';

// ── Original references captured before any patching ─────────
const _createElement    = Document.prototype.createElement;
const _addEventListener = EventTarget.prototype.addEventListener;

// ── Event types that signal a media element is worth tracking ─
const MEDIA_EVENTS = new Set([
    'play', 'playing', 'pause', 'ended',
    'loadeddata', 'loadedmetadata', 'canplay', 'canplaythrough',
    'timeupdate', 'ratechange', 'volumechange', 'emptied', 'waiting',
]);

// ── Event types blocked by Permissions-Policy on some pages ──
// Calling the native addEventListener for these on a restricted
// document causes "Permissions policy violation: unload is not
// allowed in this document." We short-circuit them so the
// original listener is never forwarded to the browser engine.
const BLOCKED_EVENT_TYPES = new Set(['unload', 'beforeunload']);

export function applyIntercepts() {

    // ── 1. Intercept createElement to catch React/Vue/Angular media ──
    Document.prototype.createElement = function createElement(tag) {
        const el = _createElement.apply(this, arguments);
        if (typeof tag === 'string') {
            const t = tag.toLowerCase();
            if (t === 'video' || t === 'audio') captureMedia(el);
        }
        return el;
    };

    // ── 2. Intercept addEventListener ────────────────────────────────
    const wrappedAddEventListener = function addEventListener(type) {
        // ① Block policy-restricted event types — never reach native code.
        if (typeof type === 'string' && BLOCKED_EVENT_TYPES.has(type.toLowerCase())) {
            console.debug(
                `WatchSmart: Suppressed addEventListener("${type}") — ` +
                'blocked by Permissions-Policy on this page.'
            );
            return undefined;
        }

        // ② Only run captureMedia when a media-related event is registered
        //    on an HTMLMediaElement, to avoid the overhead on every single
        //    addEventListener call on the page.
        if (
            this instanceof HTMLMediaElement &&
            typeof type === 'string' &&
            MEDIA_EVENTS.has(type.toLowerCase())
        ) {
            captureMedia(this);
        }

        // ③ Forward all other calls normally, logging any real errors.
        try {
            return _addEventListener.apply(this, arguments);
        } catch (e) {
            console.debug('WatchSmart: addEventListener failed', type, e);
        }
    };

    // Preserve the original function name so DevTools shows "addEventListener"
    Object.defineProperty(wrappedAddEventListener, 'name', { value: 'addEventListener' });
    // Make toString() look like the native function to avoid site fingerprinting
    Object.defineProperty(wrappedAddEventListener, 'toString', {
        value: () => 'function addEventListener() { [native code] }',
        writable: true,
        configurable: true,
    });

    EventTarget.prototype.addEventListener = wrappedAddEventListener;
}
