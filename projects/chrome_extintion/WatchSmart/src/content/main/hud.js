// hud.js — Speed badge clipped inside the media frame (never renders outside it).

import { state } from './state.js';

let hudTimer = null;
let overlayEl = null;   // clipping wrapper — same size as the media

/** Returns the largest connected tracked media element. */
function getActiveMedia() {
    let best = null, bestArea = 0;
    for (const m of state.trackedMedia) {
        if (!m.isConnected) continue;
        const r = m.getBoundingClientRect();
        const area = r.width * r.height;
        if (area > bestArea) { bestArea = area; best = m; }
    }
    return best;
}

/** Create (once) the overlay + inner badge. */
function ensureOverlay(container) {
    if (overlayEl && overlayEl.isConnected) return overlayEl;

    // ── Outer overlay: exactly covers the media, clips children ──
    overlayEl = document.createElement('div');
    overlayEl.id = 'watchsmart-overlay';
    Object.assign(overlayEl.style, {
        position:      'fixed',
        overflow:      'hidden',        // Keep inside the frame
        pointerEvents: 'none',
        zIndex:        '2147483647',
        opacity:       '0',
        transition:    'opacity 0.25s ease',
    });

    // ── Inner badge ──
    const badge = document.createElement('div');
    badge.id = 'watchsmart-hud';
    Object.assign(badge.style, {
        position:        'absolute',
        top:             '14px',
        right:           '14px',
        backgroundColor: 'rgba(15,23,42,0.92)',
        color:           '#818cf8',
        padding:         '6px 14px',
        borderRadius:    '10px',
        fontFamily:      '"Outfit",sans-serif',
        fontSize:        '18px',
        fontWeight:      '800',
        border:          '1px solid rgba(129,140,248,0.4)',
        boxShadow:       '0 4px 16px rgba(0,0,0,0.45)',
        transform:       'scale(0.85)',
        transition:      'transform 0.2s cubic-bezier(0.34,1.56,0.64,1)',
        display:         'flex',
        alignItems:      'baseline',
        userSelect:      'none',
    });

    const val  = document.createElement('span'); val.id = 'watchsmart-hud-val';
    const unit = document.createElement('span'); unit.textContent = 'x';
    Object.assign(unit.style, { fontSize: '14px', marginLeft: '2px', opacity: '0.75' });
    badge.append(val, unit);
    overlayEl.appendChild(badge);
    container.appendChild(overlayEl);
    return overlayEl;
}

export function showHUD(speed) {
    // Don't show if no tracked media
    if (!state.trackedMedia.size) return;
    const media = getActiveMedia();
    if (!media) return;

    const container = document.body || document.documentElement;
    if (!container) { setTimeout(() => showHUD(speed), 50); return; }

    const ov = ensureOverlay(container);

    // Apply media dimensions to the overlay
    const r = media.getBoundingClientRect();
    Object.assign(ov.style, {
        top:    `${r.top}px`,
        left:   `${r.left}px`,
        width:  `${r.width}px`,
        height: `${r.height}px`,
    });

    // Update number
    const valEl = ov.querySelector('#watchsmart-hud-val');
    if (valEl) {
        const p = Math.abs(speed * 10 - Math.round(speed * 10)) < 0.001 ? 1 : 2;
        valEl.innerText = speed.toFixed(p);
    }

    // Show HUD
    const badge = ov.querySelector('#watchsmart-hud');
    clearTimeout(hudTimer);
    ov.style.opacity     = '1';
    if (badge) badge.style.transform = 'scale(1)';

    hudTimer = setTimeout(() => {
        ov.style.opacity = '0';
        if (badge) badge.style.transform = 'scale(0.85)';
    }, 3000);
}
