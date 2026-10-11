# DecisionHelper

**Decision Making Helper** — Live Web-App demo, written in C++ and Qt, compiled to WebAssembly.

---

## 🚀 Live Demo

Experience the live WebAssembly demo in your browser:

> 🌐 **[mesw.github.io/DecisionHelper](https://mesw.github.io/DecisionHelper)**

*(No installation or plugins required — runs directly in any modern web browser via WebAssembly).*

---

## 🎯 About

**DecisionHelper** helps you make structured, confident decisions by breaking them down into bite-sized, pairwise micro-decisions.

Instead of struggling to balance multiple complex variables all at once, you evaluate:
1. **Two options at a time** against one specific criterion.
2. **Two criteria at a time** to determine their relative importance.

The app uses transparent pairwise scoring (Win: 1.0, Tie: 0.5, Loss: 0.0) combined with normalized criteria weights to rank your options with clear $0\% - 100\%$ scores and point breakdowns.

---

## 📋 The 6-Step Decision Workflow

1. **Step 1: Name Your Decision** — Set the title of the decision (e.g. *"Choosing a new job"*, *"Selecting a framework"*).
2. **Step 2: Enter Options** — Add two or more alternatives to choose between.
3. **Step 3: Define Criteria** — Enter the factors that matter to you (e.g. *Salary*, *Work-Life Balance*, *Growth*).
4. **Step 4: Micro-Decisions** — Compare 2 options at a time per criterion with three balanced choices:
   - `[ Option A ]`
   - `[ Equal / Tie ]`
   - `[ Option B ]`
5. **Step 5: Weight Criteria** — Compare pairs of criteria to prioritize what matters most (supports equal weighting).
6. **Step 6: Results & Analysis** — View the ranked scoreboard with transparent percentage scores, criterion breakdowns, and options to adjust any decision or add more criteria/options dynamically.

---

## 🛠️ Tech Stack

- **Language:** C++17
- **UI Framework:** Qt6 & QtQuick (QML, QtQuick Controls 2, Material Style)
- **Build System:** CMake
- **Platforms:**
  - **WebAssembly** (Qt 6.8+ single-threaded WASM via Emscripten; bundles `NotoColorEmoji.ttf` for browser emoji support)
  - **Desktop** (Windows MinGW 64-bit / MSVC / Linux / macOS)
- **Persistence:** Local JSON file on Desktop / `localStorage` on WebAssembly

---

## 💻 Building Locally in Qt Creator

1. Open `CMakeLists.txt` in **Qt Creator**.
2. Select your target kit:
   - **WebAssembly Qt 6.8.x single-threaded** (for web build)
   - **Desktop Qt 6.8.x MinGW 64-bit / GCC / Clang** (for desktop)
3. Build and Run via standard Qt Creator shortcuts (`Ctrl+B` / `Ctrl+R`).

---

## 🌐 Deploying to GitHub Pages

1. Build the **WebAssembly** target in Qt Creator.
2. The runtime files (`index.html`, `index.js`, `index.wasm`, `qtloader.js`, `qtlogo.svg`) are generated into the build folder and whitelisted by `.gitignore`.
3. In your GitHub repository settings:
   - Go to **Settings $\rightarrow$ Pages**
   - Under **Build and deployment $\rightarrow$ Source**, select **Deploy from a branch**
   - Set **Branch:** `master` (or `main`) and **Folder:** `/ (root)`
4. Push your changes:
   ```bash
   git add .
   git commit -m "Deploy latest WebAssembly build"
   git push origin master
   ```
   The root `index.html` automatically serves and redirects visitors to the application.
