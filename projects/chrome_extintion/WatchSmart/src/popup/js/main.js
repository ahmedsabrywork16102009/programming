// main.js — Popup entry point. Imports and boots all modules.

import { initSpeedController }     from './speed/index.js';
import { initToggle }              from './toggle.js';
import { initSettings }            from './settings.js';

function initPopup() {
    initToggle();
    initSettings();
    initSpeedController();
}

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initPopup);
} else {
    initPopup();
}
