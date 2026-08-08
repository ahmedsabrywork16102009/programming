// apply.js — Core speed state and applySpeed function.
// All other speed modules import from here.

import { round }                from '../utils/round.js';
import { MIN_SPEED, MAX_SPEED } from '../constants.js';
import { formatNumber, parseLocalizedNumber } from '../i18n.js';

export let currentSpeed = 1.0;

export function applySpeed(rawSpeed) {
    const lang = document.documentElement.lang || 'en';
    const cleanSpeed = parseLocalizedNumber(rawSpeed, lang);
    const speed = Math.max(MIN_SPEED, Math.min(MAX_SPEED, parseFloat(cleanSpeed)));
    if (isNaN(speed)) return;

    const fixed  = round(speed, 2);
    currentSpeed = fixed;

    // Sync slider
    const slider = document.getElementById('main-slider');
    if (slider) {
        slider.value = fixed;
        slider.style.setProperty('--val', `${(fixed / MAX_SPEED) * 100}%`);
    }

    // Sync number input (skip when user is actively typing)
    const inp = document.getElementById('current-speed-input');
    if (inp && document.activeElement !== inp) {
        const lang = document.documentElement.lang || 'en';
        const formatted = formatNumber(fixed, lang);
        inp.value = formatted;
        inp.style.width = `${formatted.length}ch`;
    }

    // Sync hidden helper for storage
    const speedVal = document.getElementById('speed-val');
    if (speedVal) speedVal.textContent = fixed;

    // Highlight matching preset
    document.querySelectorAll('.preset-btn').forEach(btn => {
        btn.classList.toggle('active', Math.abs(parseFloat(btn.dataset.speed) - fixed) < 0.05);
    });

    // Persist — content scripts listen to this storage key
    chrome.storage.local.set({ globalVideoSpeed: String(fixed) });
}

// Allow i18n module to trigger a redraw when language changes
window.addEventListener('update-speed-ui', (e) => {
    applySpeed(e.detail.speed);
});
