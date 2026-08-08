// loop.js — requestAnimationFrame loop that enforces speed at ~60fps.

import { state }          from './state.js';
import { enforceOnMedia, captureMedia } from './tracker.js';

let lastFullScan = 0;
let lastFrameTime = 0;

function quantumLoop(timestamp) {
    if (lastFrameTime === 0) lastFrameTime = timestamp;
    const dt = Math.min(timestamp - lastFrameTime, 100); // Max 100ms delta to prevent massive jumps on lag
    lastFrameTime = timestamp;

    state.trackedMedia.forEach(m => enforceOnMedia(m, dt));

    // Full DOM scan once per second to catch any newly added media
    if (timestamp - lastFullScan > 1000) {
        lastFullScan = timestamp;
        document.querySelectorAll('video, audio').forEach(captureMedia);
    }

    requestAnimationFrame(quantumLoop);
}

export function startQuantumLoop() {
    requestAnimationFrame(quantumLoop);
}
