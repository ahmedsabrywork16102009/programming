# Development Style Guide

Update this document whenever a feature is added or changed.

## Project Structure

- `manifest.json`: extension metadata, settings, and permissions.
- `src/content.js`: main DOM logic.
- `src/popup/`: popup HTML, CSS, and JavaScript.
- `assets/icons/`: extension icons.
- `docs/`: project documentation.
- When moving a file, update its path in `manifest.json`.

## Development Standards

- Use Manifest V3 only.
- Keep JavaScript outside HTML files.
- Use `MutationObserver` for React SPA updates with a 200 ms debounce.
- Do not use `setInterval`.
- Write to storage only when data changes.
- Update this document with every feature.

## Storage

- `chrome.storage.local`: stores extension settings and manual classifications.
- `localStorage`: stores course units, lessons, completion state, and UI preferences.
