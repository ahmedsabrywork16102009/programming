// keyboard.js — Global keyboard shortcuts: Ctrl+Shift+Alt + Arrow / R.

import { applyGlobalSpeed, currentSpeed, settings } from './core.js';

export function initKeyboard() {
    function sendVideoCommand(command, value) {
        window.postMessage({
            type: 'WATCHSMART_VIDEO_CMD',
            command,
            value
        }, '*');
    }

    document.addEventListener('keydown', (e) => {
        const tag = e.target.tagName.toLowerCase();
        if (tag === 'input' || tag === 'textarea' || e.target.isContentEditable) return;
        if (!e.ctrlKey || !e.shiftKey || !e.altKey) return;

        const key      = e.key;
        const keyLower = key ? key.toLowerCase() : '';
        const code     = e.code;
        
        if (code === 'KeyR' || keyLower === 'r') {
            e.preventDefault();
            applyGlobalSpeed(settings.defaultSpeed || 1.0);
            return;
        }

        // Max Speed: Code 'KeyM' OR specific keys (m, M)
        if (code === 'KeyM' || keyLower === 'm') {
            e.preventDefault();
            applyGlobalSpeed(settings.adSpeed || 16.0);
            return;
        }

        // Min Speed: Code 'KeyS' OR specific keys (s, S)
        if (code === 'KeyS' || keyLower === 's') {
            e.preventDefault();
            applyGlobalSpeed(settings.minSpeed || 0.1);
            return;
        }

        // Speed Up / Down: Arrow keys
        if (keyLower === 'arrowup'   || keyLower === 'arrowright') { e.preventDefault(); applyGlobalSpeed(parseFloat((currentSpeed + settings.speedStep).toFixed(2))); return; }
        if (keyLower === 'arrowdown' || keyLower === 'arrowleft')  { e.preventDefault(); applyGlobalSpeed(parseFloat((currentSpeed - settings.speedStep).toFixed(2))); return; }

        // --- Video Controls (all require Ctrl+Shift+Alt) ---

        // Extra Large Forward (+60s): Code 'Quote' OR specific keys
        if (code === 'Quote' || key === "'" || key === '"') {
            e.preventDefault();
            sendVideoCommand('seek', settings.seekExtra || 60);
            return;
        }

        // Large Forward (+30s): Code 'Semicolon' OR specific keys
        if (code === 'Semicolon' || key === ';' || key === ':') {
            e.preventDefault();
            sendVideoCommand('seek', settings.seekLarge || 30);
            return;
        }

        // Medium Forward (+10s): Code 'KeyL' OR specific keys (l, L)
        if (code === 'KeyL' || keyLower === 'l') {
            e.preventDefault();
            sendVideoCommand('seek', settings.seekMedium || 10);
            return;
        }

        // Small Forward (+5s): Code 'KeyK' OR specific keys (k, K)
        if (code === 'KeyK' || keyLower === 'k') {
            e.preventDefault();
            sendVideoCommand('seek', settings.seekSmall || 5);
            return;
        }

        // Play/Pause: Code 'KeyJ' OR specific keys (j, J)
        if (code === 'KeyJ' || keyLower === 'j') {
            e.preventDefault();
            sendVideoCommand('togglePlayPause');
            return;
        }

        // Small Backward (-5s): Code 'KeyH' OR specific keys (h, H)
        if (code === 'KeyH' || keyLower === 'h') {
            e.preventDefault();
            sendVideoCommand('seek', -(settings.seekSmall || 5));
            return;
        }

        // Medium Backward (-10s): Code 'KeyG' OR specific keys (g, G)
        if (code === 'KeyG' || keyLower === 'g') {
            e.preventDefault();
            sendVideoCommand('seek', -(settings.seekMedium || 10));
            return;
        }

        // Large Backward (-30s): Code 'KeyF' OR specific keys (f, F)
        if (code === 'KeyF' || keyLower === 'f') {
            e.preventDefault();
            sendVideoCommand('seek', -(settings.seekLarge || 30));
            return;
        }

        // Extra Large Backward (-60s): Code 'KeyD' OR specific keys (d, D)
        if (code === 'KeyD' || keyLower === 'd') {
            e.preventDefault();
            sendVideoCommand('seek', -(settings.seekExtra || 60));
            return;
        }
    }, true);
}