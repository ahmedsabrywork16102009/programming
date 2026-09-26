// ad-skipper.js — Instant ad elimination: quality drop + internal API + seek + DOM click + overlay purge.

import { state }       from './state.js';
import { isAdPlaying } from './ad-detect.js';

// ── Anti-adblock overlay selectors (S5) ──────────────────────────────
// YouTube's enforcement popups that pause the video and demand AdBlock
// to be disabled. We remove them from the DOM the instant they appear.
const OVERLAY_SELECTORS = [
    'tp-yt-paper-dialog',                   // Generic YT modal
    'ytd-enforcement-message-view-model',   // "Ad blocker detected" card
    '.ytd-enforcement-message-view-model',
    'ytd-ad-block-survey-renderer',         // Survey popup
    '.ytp-ad-overlay-close-button',         // Ad overlay close
    '.ytp-ad-text-overlay',                 // Text overlay ads
    '.ytp-ad-image-overlay',                // Image overlay ads
    '[id="enforcement-dialog"]',            // Enforcement dialog
    '[class*="enforcement"]',               // Any enforcement class
    'ytd-popup-container tp-yt-paper-dialog', // Nested popup
].join(', ');

// ── Skip button selectors (YouTube modern/classic + generic) ──────────
const SKIP_SELECTORS = [
    '.ytp-ad-skip-button-modern',
    '.ytp-ad-skip-button-modern .ytp-ad-skip-button-slot',
    '.ytp-skip-ad-button',
    '.ytp-ad-skip-button',
    '.ytp-ad-skip-button-container button',
    '.ytp-ad-skip-button-text',
    '[class*="skip-ad"]',
    '[class*="skipAd"]',
    '[class*="skip_ad"]',
    '[id*="skip-ad"]',
    '[id*="skipAd"]',
    '[aria-label*="Skip"]',
    '[data-testid*="skip"]',
].join(', ');

// ── State ──────────────────────────────────────────────────────────────
let _observer      = null;
let _lastSkipTime  = 0;
let _savedQuality  = null;   // Quality before ad (restored after)
let _qualityLowered = false;
let _wasInAd       = false;

// ── YouTube player reference ──────────────────────────────────────────
function getPlayer() {
    return /** @type {any} */ (document.getElementById('movie_player'));
}

// ── S1: Drop ad quality to lowest to save bandwidth ───────────────────
function lowerAdQuality() {
    if (_qualityLowered) return;
    const p = getPlayer();
    if (!p) return;
    try {
        if (typeof p.getPlaybackQuality === 'function') {
            _savedQuality = p.getPlaybackQuality();
        }
        // setPlaybackQualityRange pins both min and max to 'tiny' (144p)
        if (typeof p.setPlaybackQualityRange === 'function') {
            p.setPlaybackQualityRange('tiny', 'tiny');
        } else if (typeof p.setPlaybackQuality === 'function') {
            p.setPlaybackQuality('tiny');
        }
        _qualityLowered = true;
    } catch (e) {
        console.debug('WatchSmart: quality lower failed', e);
    }
}

// ── Restore quality when the ad ends ──────────────────────────────────
function restoreQuality() {
    if (!_qualityLowered) return;
    const p = getPlayer();
    if (!p) return;
    try {
        const q = _savedQuality || 'auto';
        if (typeof p.setPlaybackQualityRange === 'function') {
            p.setPlaybackQualityRange('tiny', q);   // Restore full range
        } else if (typeof p.setPlaybackQuality === 'function') {
            p.setPlaybackQuality(q);
        }
    } catch {}
    _qualityLowered = false;
    _savedQuality   = null;
}

// ── S2: YouTube internal skipAd() ─────────────────────────────────────
function tryInternalSkip(player) {
    try {
        if (typeof player?.skipAd === 'function') {
            player.skipAd();
            return true;
        }
    } catch {}
    return false;
}

// ── S3: Seek video to end (works when ad is seekable) ─────────────────
function trySeekToEnd(video) {
    try {
        if (video && isFinite(video.duration) && video.duration > 0) {
            if (video.currentTime < video.duration - 0.1) {
                video.currentTime = video.duration;
                return true;
            }
        }
    } catch {}
    return false;
}

// ── S4: DOM button click (fallback) ───────────────────────────────────
function tryDomClick() {
    try {
        const btn = document.querySelector(SKIP_SELECTORS);
        if (btn) { btn.click(); return true; }
    } catch {}
    return false;
}

// ── S5: Anti-adblock overlay purger ───────────────────────────────────
// Removes "Ad blocker detected" enforcement popups/dialogs from the DOM
// the instant they appear, then resumes the video if it was paused.
function clearOverlays() {
    try {
        let removed = false;
        document.querySelectorAll(OVERLAY_SELECTORS).forEach(el => {
            // Only remove overlays that look like enforcement / ad overlay
            // (not generic dialogs the user opened themselves)
            if (el.offsetParent !== null || el.style.display !== 'none') {
                el.remove();
                removed = true;
            }
        });

        // If a popup paused the video, resume it
        if (removed) {
            const video = document.querySelector('video');
            if (video && video.paused) {
                video.play().catch(() => {});
            }
        }
    } catch {}
}

// ── Master trySkip — runs every frame from loop.js ────────────────────
export function trySkip() {
    if (state.settings?.autoSkipAds === false) return;

    const now = Date.now();
    if (now - _lastSkipTime < 80) return;   // ~12 checks/sec is enough

    const adActive = isAdPlaying(null);

    if (adActive) {
        _wasInAd = true;

        // S1 — kill bandwidth immediately
        lowerAdQuality();

        const player = getPlayer();
        const video  = document.querySelector('video');

        let skipped = false;
        // S2 — internal API (most reliable when available)
        skipped = tryInternalSkip(player) || skipped;
        // S3 — seek to end (instant, no buffering of remaining frames)
        skipped = trySeekToEnd(video) || skipped;
        // S4 — click the DOM button (guarantees the UI is dismissed)
        skipped = tryDomClick() || skipped;

        if (skipped) _lastSkipTime = now;

    } else {
        // S5 — always scan for enforcement overlays (even outside ad playback)
        clearOverlays();

    }

    if (_wasInAd && !adActive) {
        // Ad just ended — restore video quality
        _wasInAd = false;
        restoreQuality();
    }
}

// ── MutationObserver for 0ms event-driven triggers ────────────────────
function startObserver() {
    if (_observer) return;
    _observer = new MutationObserver((mutations) => {
        // Check for added nodes first — enforcement popups are added nodes
        for (const m of mutations) {
            if (m.type === 'childList' && m.addedNodes.length > 0) {
                clearOverlays(); // S5 — instant popup purge on new node
            }
        }
        trySkip();
    });
    _observer.observe(document.body || document.documentElement, {
        childList: true, subtree: true,
        attributes: true,
        attributeFilter: ['class', 'style', 'disabled', 'aria-hidden'],
    });
}

function stopObserver() {
    _observer?.disconnect();
    _observer = null;
}

// ── Public API (called from bridge.js + index.js) ─────────────────────
export function syncSkipperSettings() {
    if (state.settings.autoSkipAds !== false) {
        startObserver();
        trySkip();
    } else {
        stopObserver();
        restoreQuality();   // Safety: restore if user disables mid-ad
    }
}

export function initAdSkipper() {
    if (state.settings.autoSkipAds !== false) {
        startObserver();
    }
}
