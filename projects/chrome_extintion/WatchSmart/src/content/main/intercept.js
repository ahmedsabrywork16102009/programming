// intercept.js — Intercept dynamic media creation to capture React/Vue elements.

import { captureMedia } from './tracker.js';

const _createElement   = Document.prototype.createElement;
const _addEventListener = EventTarget.prototype.addEventListener;

export function applyIntercepts() {
    Document.prototype.createElement = function (tag) {
        const el = _createElement.apply(this, arguments);
        if (typeof tag === 'string') {
            const t = tag.toLowerCase();
            if (t === 'video' || t === 'audio') captureMedia(el);
        }
        return el;
    };

    EventTarget.prototype.addEventListener = function (type) {
        if (this instanceof HTMLMediaElement) captureMedia(this);
        try {
            return _addEventListener.apply(this, arguments);
        } catch (e) {
            // Silently swallow Permissions-Policy violations (e.g. 'unload' on restricted pages)
        }
    };
}
