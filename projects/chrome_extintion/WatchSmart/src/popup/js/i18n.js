// i18n.js — Internationalization system for WatchSmart (English Only)

export const translations = {
    en: {
        dir: 'ltr',
        locale: 'en-US',
        langName: 'English',
        appName: 'WatchSmart',
        settings: 'Settings',
        speedLimits: 'Speed Limits & Steps',
        defaultSpeed: 'Default Speed (R)',
        defaultSpeedTip: 'The speed the player resets to when you press R.',
        minSpeed: 'Min Speed (S)',
        minSpeedTip: 'The lowest speed limit when using shortcuts.',
        maxAdSpeed: 'Max Ad Speed (M)',
        maxAdSpeedTip: 'Maximum speed for ads.',
        step: 'Keyboard/Mouse Step',
        stepTip: 'How much the speed changes when using mouse wheel or arrow keys.',
        seekSteps: 'Video Seek Steps',
        smallJump: 'Small Jump (s)',
        smallJumpTip: 'Jump forward/backward using K or H.',
        mediumJump: 'Medium Jump (s)',
        mediumJumpTip: 'Jump forward/backward using L or G.',
        largeJump: 'Large Jump (s)',
        largeJumpTip: 'Jump forward/backward using ; or F.',
        extraJump: 'Extra Large Jump (s)',
        extraJumpTip: "Jump forward/backward using ' or D.",
        visuals: 'Visuals & HUD',
        showHud: 'Show Speed HUD',
        showHudTip: 'Show large number on screen when speed changes.',
        save: 'Save Settings',
        saved: 'Saved!',
        back: 'Back',
        toggleExt: 'Enable/Disable extension',
        decreaseSpeed: 'Decrease speed',
        increaseSpeed: 'Increase speed'
    }
};

export function formatNumber(num, lang = 'en') {
    const locale = translations[lang]?.locale || 'en-US';
    return new Intl.NumberFormat(locale).format(num);
}

export function parseLocalizedNumber(str, lang = 'en') {
    if (!str) return '0';
    return String(str).replace(/[^0-9.]/g, '');
}

export function applyLanguage(lang = 'en') {
    const dict = translations[lang] || translations.en;
    
    document.body.dir = dict.dir;
    document.documentElement.lang = lang;

    document.querySelectorAll('[data-i18n]').forEach(el => {
        const key = el.getAttribute('data-i18n');
        if (dict[key]) {
            el.textContent = dict[key];
        }
    });

    document.querySelectorAll('[data-i18n-title]').forEach(el => {
        const key = el.getAttribute('data-i18n-title');
        if (dict[key]) {
            el.setAttribute('title', dict[key]);
        }
    });

    document.querySelectorAll('[data-i18n-placeholder]').forEach(el => {
        const key = el.getAttribute('data-i18n-placeholder');
        if (dict[key]) {
            el.setAttribute('placeholder', dict[key]);
        }
    });
}
