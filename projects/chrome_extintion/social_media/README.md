# FocusBlock - Website Blocker Chrome Extension

FocusBlock is a premium, modern Chrome Extension designed to boost productivity by restricting access to distracting websites using custom Whitelist and Blacklist modes.

## Features ✨

- **Blacklist Mode:** Restrict specific distracting websites (e.g., social media). All other websites remain accessible.
- **Whitelist Mode:** Block all websites on the internet except a selected few approved sites (perfect for deep work).
- **Modern Premium Design:** A clean interface featuring light and dark modes with glassmorphic cards and glowing highlights.
- **Robust Redirection Engine:** Automatically redirects blocked requests to a clean, focused custom warning page.
- **Storage Synchronization:** Remembers your configuration, selected theme, and websites list persistently.
- **Internal Bypass:** Safe architecture that automatically permits Chrome settings, local workspace tools, and extension configuration so the browser never crashes.

## Folder Directory Structure 📁

```
├── manifest.json      # Extension metadata and config
├── assets/
│   └── logo.png       # App logo & icons
├── background/
│   └── background.js  # Service worker handling block/redirection logic
├── popup/
│   ├── popup.html     # Control dashboard UI
│   ├── popup.js       # Toggle switches, input control, storage sync
│   └── popup.css      # Custom styling, dark/light themes, animations
└── blocked/
    ├── blocked.html   # Access denied landing page
    ├── blocked.js     # Blocked site details and navigation logic
    └── blocked.css    # Premium style for blocked page
```

## How to Install 🚀

1. Open **Google Chrome**.
2. Navigate to `chrome://extensions/`.
3. Enable **Developer mode** (toggle in the upper-right corner).
4. Click **Load unpacked** in the upper-left corner.
5. Select the main project folder (`social_media`).

## How to Use 💻

### Adding a Site:
1. Click the **FocusBlock** extension icon in your toolbar.
2. Enter the domain name (e.g., `facebook.com` or `wikipedia.org`).
3. Click the **Add** button.

### Switching Modes:
1. Open the extension popup.
2. Toggle between **Blacklist** and **Whitelist** modes.
3. The UI color palette and blocking rules will instantly update.

### Deleting a Site:
1. Click **Delete** next to any domain name in the list.

### Clearing the List:
1. Click the **Clear List** button at the bottom of the popup.
2. Confirm the action.

## Permissions Requested 🔐

- `storage`: Persists blocking lists, active modes, and theme selections.
- `tabs`: Handles page state updates and tab redirection.

---

**Developed with ❤️ to help you focus.**
