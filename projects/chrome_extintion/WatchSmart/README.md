# 🚀 WatchSmart: Documentation & Technical Overview

WatchSmart is a professional-grade video speed controller for Chrome, specifically engineered to bypass anti-cheat mechanisms and provide a seamless, high-performance experience across all video platforms (YouTube, Netflix, Coursera, etc.).

---

## 🛠 Core Features

### 1. High-Performance Speed Control
- **Range**: From **0.1x** to **16.0x**.
- **Precision**: Steps as small as **0.01x**.
- **60FPS Enforcement**: Unlike other extensions that "lag" or drop frames when speeding up, WatchSmart uses a **Quantum Loop** system to enforce the desired speed every frame (approx. 16.6ms).

### 2. Anti-Cheat & Site Bypass
- **World Switching**: Uses the Chrome `MAIN` world to inject logic before site scripts can detect or block speed changes.
- **Native Interception**: Overrides the native `HTMLVideoElement.prototype.playbackRate` to prevent sites from resetting your preferred speed.
- **Dynamic Detection**: Automatically detects and applies speed to new video elements added dynamically (React/Vue/Angular).

### 3. Smart Ad Handling
- **Ad Detection**: Identifies when an advertisement is playing.
- **Max Ad Speed**: Automatically boosts ads to **16x** (or your preferred setting) and can optionally mute them, returning to your normal speed once the video resumes.

### 4. Professional UX & HUD
- **On-Screen HUD**: A sleek, non-intrusive number appears on the video when speed changes.
- **Exclusive Settings**: Modern accordion-style settings menu that keeps your workspace clean.
- **Responsive Controls**:
    - **Keyboard**: Arrow Up/Down for fine steps, 'R' for reset.
    - **Mouse**: Wheel control over the video or the popup.
    - **Scrubbing**: Click and drag the speed display for fast adjustments.

---

## ⌨️ Shortcuts & Controls

| Action | Control |
| :--- | :--- |
| **Increase Speed** | `Arrow Up` / `Arrow Right` / `Mouse Wheel Up` |
| **Decrease Speed** | `Arrow Down` / `Arrow Left` / `Mouse Wheel Down` |
| **Reset to Default** | `R` Key |
| **Small Seek Jump** | `K` / `H` |
| **Medium Seek Jump** | `L` / `G` |
| **Large Seek Jump** | `;` / `F` |
| **Extra Large Jump** | `'` / `D` |

---

## 📁 Project Structure (Cleaned)

```text
WatchSmart/
├── dist/               # Built extension (Load this into Chrome)
├── src/
│   ├── background/     # Extension background logic
│   ├── content/
│   │   ├── isolated/   # Keyboard/Wheel/Storage logic
│   │   └── main/       # Anti-cheat/Speed enforcement logic
│   └── popup/
│       ├── css/        # Styling (Modern Dark Theme)
│       └── js/         # Popup interaction logic
├── manifest.json       # Extension configuration
└── build.js            # esbuild-based build system
```

---

## 🚀 Installation for Developers
1. Run `npm install` to install dependencies (esbuild).
2. Run `npm run build` to generate the `dist` folder.
3. Open `chrome://extensions`, enable **Developer Mode**, and click **Load Unpacked**.
4. Select the `dist` folder.

---

> [!NOTE]
> This extension is optimized for **English** and follows a minimalist design philosophy, removing all unnecessary cloud/storage bloat to ensure maximum performance.
