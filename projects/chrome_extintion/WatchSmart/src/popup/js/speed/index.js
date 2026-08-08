// index.js — Speed controller bootstrap. Wires all UI to applySpeed.

import { applySpeed, currentSpeed } from './apply.js';
import { initScrubbing }            from './scrubbing.js';
import { initSpeedKeyboard }        from './keyboard.js';
import { round }                    from '../utils/round.js';
import { STEP }                     from '../constants.js';


export function initSpeedController() {
    const slider       = document.getElementById('main-slider');
    const speedInput   = document.getElementById('current-speed-input');
    const speedDownBtn = document.getElementById('speed-down-btn');
    const speedUpBtn   = document.getElementById('speed-up-btn');
    const presetBtns   = document.querySelectorAll('.preset-btn');

    if (!slider || !speedInput) return;

    // Restore saved speed on open
    chrome.storage.local.get(['globalVideoSpeed'], (res) => {
        applySpeed(res.globalVideoSpeed || '1.0');
    });

    // +/− buttons
    speedDownBtn?.addEventListener('click', () => applySpeed(parseFloat((currentSpeed - STEP).toFixed(2))));
    speedUpBtn?.addEventListener('click',   () => applySpeed(parseFloat((currentSpeed + STEP).toFixed(2))));

    // Manual text input
    speedInput.addEventListener('change',  () => applySpeed(speedInput.value));
    speedInput.addEventListener('keydown', (e) => {
        if (e.key === 'Enter') { speedInput.blur(); applySpeed(speedInput.value); }
    });

    // Slider & Widget Wheel Control
    slider.addEventListener('input',  () => applySpeed(slider.value));
    
    function handleWheel(e) {
        e.preventDefault();
        applySpeed(parseFloat((currentSpeed + (e.deltaY > 0 ? -STEP : STEP)).toFixed(2)));
    }
    
    slider.addEventListener('wheel', handleWheel, { passive: false });
    
    const widget = document.querySelector('.speed-control-widget');
    if (widget) {
        widget.addEventListener('wheel', handleWheel, { passive: false });
    }

    // Preset buttons
    presetBtns.forEach(btn => btn.addEventListener('click', () => applySpeed(btn.dataset.speed)));

    // Sub-features
    initScrubbing();
    initSpeedKeyboard();
}
