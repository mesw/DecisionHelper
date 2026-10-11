# GEMINI.md

This file provides guidance to Gemini when working with code in this repository.

## Project Overview

**DecisionHelper** — a cross-platform Qt6 application (targeting Desktop & WebAssembly via Qt Creator) that helps users make decisions through structured pairwise micro-decisions.

The core philosophy is **simplicity and clarity**: breaking a decision into small, bite-sized pairwise comparisons per criterion, weighting criteria via pairwise comparison, and computing a transparent weighted result.

## Tech Stack

- **Language:** C++17
- **UI Framework:** Qt6 + QtQuick (QML, QtQuick Controls 2, Material Style)
- **Build System:** CMake
- **Target Environments:**
  - Desktop (Windows MinGW 64-bit / MSVC / Linux / macOS)
  - WebAssembly (Qt 6.8+ single-threaded WASM via Emscripten; bundles `NotoColorEmoji.ttf` resource for browser emoji rendering)
- **Primary IDE:** Qt Creator (manual build & run by user)
- **Persistence:** Local JSON file on Desktop (`QStandardPaths::AppDataLocation`) / `localStorage` on WebAssembly

## Build & Test Workflow

The user manages and tests the project directly inside **Qt Creator**:
1. Open `CMakeLists.txt` in Qt Creator.
2. Select target kit (`Desktop Qt 6.8.x MinGW 64-bit` or `WebAssembly Qt 6.8.x`).
3. Build and Run via standard Qt Creator controls (`Ctrl+R` / `Ctrl+B`).

When editing code, ensure all files compile cleanly under C++17 and Qt 6.5+ standards.

## WebAssembly & GitHub Pages Deployment

- The WebAssembly runtime files (`index.html`, `index.js`, `index.wasm`, `qtloader.js`, `qtlogo.svg`) are generated into the build folder (e.g. `build/WebAssembly_Qt_6_8_3_single_threaded-Debug/`).
- `.gitignore` whitelists only these 5 runtime files from the WASM build directory; all intermediate build files (`.ninja*`, `*.o`, `*.cpp`, `CMakeCache.txt`, etc.) and Desktop builds are ignored.
- The root [index.html](file:///c:/Users/MiCHi/Documents/decisionHelper/index.html) automatically forwards visitors to the WebAssembly build folder.
- Deployment to GitHub Pages can be configured either via:
  - **Deploy from a branch:** Set **Settings > Pages > Source: Deploy from a branch**, Branch: `master` / Folder: `/ (root)`.
  - **GitHub Actions:** Set **Settings > Pages > Source: GitHub Actions** (runs `.github/workflows/deploy-pages.yml`).

## Target User Workflow (6 Phases)

1. **Phase 1: Decision Name & Setup**
   - User enters the title/name of the decision (e.g. *"Choosing a new car"*).
2. **Phase 2: Enter Options**
   - User enters the options being evaluated (e.g. *Option A, Option B, Option C*). Minimum 2 options.
3. **Phase 3: Define Criteria**
   - User enters comparison criteria (e.g. *Price, Quality, Comfort*). Minimum 1 criterion.
4. **Phase 4: Micro-Decisions (Options per Criterion)**
   - For each criterion, compare 2 options at a time.
   - **Crucial UI Rule:** Three equally sized and accentuated choices:
     - `[ Option A ]`
     - `[ Equal / Tie ]`
     - `[ Option B ]`
5. **Phase 5: Criteria Weighting (Pairwise)**
   - Compare 2 criteria at a time (e.g. *Price* vs *Quality*).
   - Allows equal weighting for multiple top priorities.
   - Three equally sized choices: `[ Criterion A ]`, `[ Equal ]`, `[ Criterion B ]`.
6. **Phase 6: Results Display**
   - Ranked options with transparent scores ($0\% - 100\%$).
   - Breakdown of points per criterion. No confusing statistics or complex regression.

## Data Model

```
DecisionSession
├── title: string
├── options: vector<Option>       // { id, label, rankPosition, score }
├── criteria: vector<Criterion>   // { id, label, weight }
├── optionComparisons: map<CriterionId, map<PairKey, int>> // +1 (A), -1 (B), 0 (Equal)
└── criteriaComparisons: map<PairKey, int>                 // +1 (A), -1 (B), 0 (Equal)
```

## Guiding Principles

- **No Over-Engineering:** Avoid complex statistical models (no Spearman correlation, Kendall tau, Fisher z-transforms, L2 regularization) unless specifically requested.
- **Consistent UI Accentuation:** The "Equal" choice must always have equal visual weight, sizing, and interaction parity with option cards.
- **Maintain Documentation Integrity:** Keep `ALGORITHMS.md`, `PAPER_PROTOTYPE.md`, and `GEMINI.md` synchronized with the actual implementation.
