// state.js — Shared mutable state for the MAIN world engine.
// All modules import this object and mutate it directly.

export const state = {
    desiredSpeed:  1.0,
    trackedMedia:  new Set(),   // <video> and <audio> elements
    enabled:       true,
    settings: {
        adSpeed: 16.0,
        showHud: true
    }
};
