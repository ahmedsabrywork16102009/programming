// wheel.js — Global scroll wheel shortcut: Ctrl+Shift+Alt + Scroll.

import { applyGlobalSpeed, currentSpeed, settings } from './core.js';

export function initWheel() {
    document.addEventListener('wheel', (e) => {
        if (!(e.ctrlKey && e.shiftKey && e.altKey)) return;
        e.preventDefault();
        applyGlobalSpeed(currentSpeed + (e.deltaY > 0 ? -settings.speedStep : settings.speedStep));
    }, { passive: false });
}
