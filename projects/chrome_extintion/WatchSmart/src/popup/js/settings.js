// settings.js — Handle UI for the settings view (English Only)

import { applyLanguage, translations } from './i18n.js';

// ── ELEMENTS ────────────────────────────────────────────────────────
const mainView        = document.getElementById('main-view');
const settingsView    = document.getElementById('settings-view');
const settingsBtn    = document.getElementById('settings-btn');
const backBtn        = document.getElementById('back-btn');
const saveBtn        = document.getElementById('save-settings-btn');

// Inputs
const inputDefaultSpeed = document.getElementById('set-default-speed');
const inputMinSpeed    = document.getElementById('set-min-speed');
const inputAdSpeed     = document.getElementById('set-ad-speed');
const inputSpeedStep   = document.getElementById('set-speed-step');

const inputSeekSmall   = document.getElementById('set-seek-small');
const inputSeekMedium  = document.getElementById('set-seek-medium');
const inputSeekLarge   = document.getElementById('set-seek-large');
const inputSeekExtra   = document.getElementById('set-seek-extra');

const inputShowHud     = document.getElementById('set-show-hud');

// ── DEFAULTS ────────────────────────────────────────────────────────
const DEFAULT_SETTINGS = {
    speedStep: 0.05,
    adSpeed: 16.0,
    defaultSpeed: 1.0,
    minSpeed: 0.1,
    seekSmall: 5,
    seekMedium: 10,
    seekLarge: 30,
    seekExtra: 60,
    showHud: true
};

// ── INITIALIZATION ──────────────────────────────────────────────────
export function initSettings() {
    // Navigation
    settingsBtn.addEventListener('click', () => {
        mainView.classList.add('hidden');
        settingsView.classList.remove('hidden');
        loadSettings();
    });

    backBtn.addEventListener('click', () => {
        settingsView.classList.add('hidden');
        mainView.classList.remove('hidden');
    });

    // Save
    saveBtn.addEventListener('click', () => saveSettings());

    // Attach wheel listeners to all number inputs
    attachWheelListeners();

    // Auto-save on any input change
    const allInputs = settingsView.querySelectorAll('input');
    allInputs.forEach(input => {
        input.addEventListener('input', () => saveSettings(true));
        input.addEventListener('change', () => saveSettings(true));
    });

    // Accordion Toggles (Exclusive mode)
    const accordionHeaders = settingsView.querySelectorAll('.accordion-header');
    accordionHeaders.forEach(header => {
        header.addEventListener('click', () => {
            const section = header.parentElement;
            const isOpen = section.classList.contains('open');

            // Close all sections first
            settingsView.querySelectorAll('.accordion-section').forEach(s => s.classList.remove('open'));

            // If it wasn't open, open it now
            if (!isOpen) {
                section.classList.add('open');
            }
        });
    });

    // Load initial settings
    loadSettings();
}

function attachWheelListeners() {
    const numberInputs = settingsView.querySelectorAll('input[type="number"]');
    numberInputs.forEach(input => {
        input.addEventListener('wheel', (e) => {
            e.preventDefault();
            const step = parseFloat(input.step) || 1;
            const currentVal = parseFloat(input.value) || 0;
            const newVal = currentVal + (e.deltaY < 0 ? step : -step);
            
            const min = input.min !== "" ? parseFloat(input.min) : -Infinity;
            const max = input.max !== "" ? parseFloat(input.max) : Infinity;
            
            const clamped = Math.max(min, Math.min(max, newVal));
            input.value = clamped % 1 === 0 ? clamped : clamped.toFixed(2);
            
            input.dispatchEvent(new Event('input'));
        }, { passive: false });
    });
}

function loadSettings() {
    chrome.storage.local.get(['extensionSettings'], (res) => {
        let rawSettings = res.extensionSettings || {};
        const settings = { ...DEFAULT_SETTINGS, ...rawSettings };
        
        inputSpeedStep.value   = settings.speedStep;
        inputAdSpeed.value     = settings.adSpeed;
        inputDefaultSpeed.value= settings.defaultSpeed;
        inputMinSpeed.value    = settings.minSpeed;

        inputSeekSmall.value   = settings.seekSmall;
        inputSeekMedium.value  = settings.seekMedium;
        inputSeekLarge.value   = settings.seekLarge;
        inputSeekExtra.value   = settings.seekExtra;

        inputShowHud.checked   = settings.showHud;

        applyLanguage('en');
    });
}

function saveSettings(isSilent = false) {
    const newSettings = {
        speedStep: parseFloat(inputSpeedStep.value) || DEFAULT_SETTINGS.speedStep,
        adSpeed: parseFloat(inputAdSpeed.value) || DEFAULT_SETTINGS.adSpeed,
        defaultSpeed: parseFloat(inputDefaultSpeed.value) || DEFAULT_SETTINGS.defaultSpeed,
        minSpeed: parseFloat(inputMinSpeed.value) || DEFAULT_SETTINGS.minSpeed,

        seekSmall: parseInt(inputSeekSmall.value) || DEFAULT_SETTINGS.seekSmall,
        seekMedium: parseInt(inputSeekMedium.value) || DEFAULT_SETTINGS.seekMedium,
        seekLarge: parseInt(inputSeekLarge.value) || DEFAULT_SETTINGS.seekLarge,
        seekExtra: parseInt(inputSeekExtra.value) || DEFAULT_SETTINGS.seekExtra,

        showHud: inputShowHud.checked
    };

    chrome.storage.local.set({ extensionSettings: newSettings }, () => {
        if (!isSilent) {
            const originalText = saveBtn.textContent;
            saveBtn.textContent = translations.en.saved;
            saveBtn.classList.add('save-success');
            setTimeout(() => {
                saveBtn.textContent = originalText;
                saveBtn.classList.remove('save-success');
            }, 1500);
        }
    });
}
