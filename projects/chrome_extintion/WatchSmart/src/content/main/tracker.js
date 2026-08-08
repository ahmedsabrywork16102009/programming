// tracker.js — Track media elements (video + audio) and enforce desired playback speed.

import { state }                     from './state.js';
import { isYouTube, nativeSet, nativeGet } from './anti-cheat.js';
import { isAdPlaying }               from './ad-detect.js';
import { showHUD }                   from './hud.js';

export function enforceOnMedia(media) {
    if (!media || !state.enabled) return;
    try {
        const adDetected = isAdPlaying(media);
        const target = adDetected ? (state.settings.adSpeed || 16.0) : state.desiredSpeed;
        
        // 1. Enforce Speed
        const actual = isYouTube ? media.playbackRate : (nativeGet ? nativeGet.call(media) : media.playbackRate);
        if (Math.abs(actual - target) > 0.001) {
            if (isYouTube) {
                media.playbackRate = target;
            } else {
                nativeSet?.call(media, target);
            }
        }

        // 2. Smart Mute (Tab-local, only affects this media element)
        if (adDetected) {
            if (!media.muted) {
                media._wsMutedByAd = true; // Mark that we muted it
                media.muted = true;
            }
        } else {
            // If ad ended and we were the ones who muted it, restore sound
            if (media._wsMutedByAd) {
                media.muted = false;
                delete media._wsMutedByAd;
            }
        }
    } catch {}
}

export function captureMedia(media) {
    if (!media || state.trackedMedia.has(media)) return;
    state.trackedMedia.add(media);
    enforceOnMedia(media);

    const onActive = () => { enforceOnMedia(media); if (state.settings.showHud !== false) showHUD(state.desiredSpeed); };
    media.addEventListener('play',       onActive, true);
    media.addEventListener('playing',    onActive, true);
    media.addEventListener('loadeddata', onActive, true);
}

// Legacy aliases for any external imports still using old names
export { enforceOnMedia as enforceOnVideo, captureMedia as captureVideo };
