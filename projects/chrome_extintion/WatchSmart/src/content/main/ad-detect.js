// ad-detect.js — Detect ads on video sites (YouTube, etc.) for speedup.

import { isYouTube } from './anti-cheat.js';

// ── YouTube Ad Detection ─────────────────────────────────────
function isYouTubeAdPlaying() {
    // YouTube puts .ad-showing on #movie_player
    const player = document.querySelector('#movie_player');
    if (player && player.classList.contains('ad-showing')) return true;

    if (document.querySelector('.ad-showing')) return true;
    if (document.querySelector('.ad-interrupting')) return true;

    // Check for ad overlay text (e.g. "Ad · 0:15")
    const adText = document.querySelector('.ytp-ad-text');
    if (adText && adText.offsetParent !== null) return true;

    return false;
}

// ── Generic (non-YouTube) Ad Detection ───────────────────────
const GENERIC_AD_SELECTORS = [
    '.ima-ad-container', '[data-testid="ad-video"]', '[id^="ad-video"]',
    '[class*="video-ad"]', '[class*="ad-video"]',
].join(', ');

function isGenericAdPlaying(video) {
    if (!video) return false;
    try { return !!video.closest(GENERIC_AD_SELECTORS); } catch { return false; }
}

// ── Combined detection ───────────────────────────────────────
export function isAdPlaying(video) {
    if (isYouTube) return isYouTubeAdPlaying();
    return isGenericAdPlaying(video);
}
