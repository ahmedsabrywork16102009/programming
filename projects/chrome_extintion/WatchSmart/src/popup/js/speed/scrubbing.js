// scrubbing.js — Click-drag on the speed display to scrub speed.

import { applySpeed, currentSpeed } from './apply.js';

export function initScrubbing() {
    const wrapper = document.querySelector('.speed-input-wrapper');
    if (!wrapper) return;

    let isDragging = false;
    let startX     = 0;
    let startSpeed = 1.0;

    wrapper.addEventListener('mousedown', (e) => {
        isDragging = true;
        startX     = e.clientX;
        startSpeed = currentSpeed;
        document.body.style.cursor = 'ew-resize';
        wrapper.classList.add('scrubbing');
    });

    window.addEventListener('mousemove', (e) => {
        if (!isDragging) return;
        // ── SENSITIVITY ─────────────────────────────────────────────────────
        // 1px = 0.005 speed units → need to drag 10px to change by 0.05
        // To increase sensitivity: raise the number (e.g. 0.01, 0.02)
        // To decrease sensitivity: lower the number (e.g. 0.003, 0.002)
        const SENSITIVITY = 0.005;
        const raw = startSpeed + (e.clientX - startX) * SENSITIVITY;
        // Round to nearest 0.05 step for clean values
        applySpeed(Math.round(raw / 0.05) * 0.05);
    });

    window.addEventListener('mouseup', () => {
        if (!isDragging) return;
        isDragging = false;
        document.body.style.cursor = '';
        wrapper.classList.remove('scrubbing');
    });
}
