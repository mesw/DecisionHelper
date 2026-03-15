# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

**DecisionHelper** — a webapp that helps users make decisions and surfaces the true reasoning behind them, including unconscious factors. Fully client-side, no backend.

## Tech Stack

- **Language:** C++17/20
- **UI:** Qt6 + QtQuick (QML, QtQuick Controls 2, QtQuick animations/gestures, Qt Charts)
- **Build:** CMake
- **Target:** WebAssembly via Emscripten (`wasm32-unknown-emscripten`)
- **Hosting:** GitHub Pages (`gh-pages` branch, static WASM deployment)
- **Persistence:** Browser `localStorage` via Emscripten JS interop
- **Analysis:** Pure C++ (rank correlation, linear regression — no server)

## Build Commands

```bash
# Configure (Qt for WebAssembly kit must be set up with Emscripten)
emcmake cmake -B build-wasm -DCMAKE_BUILD_TYPE=Release

# Build
emmake cmake --build build-wasm

# Output: build-wasm/index.html, app.wasm, app.js, qtloader.js
```

## Architecture

### User Flow (5 Phases)
1. **Add Options** — open set, extensible at any time
2. **Pairwise Ranking** — adaptive comparison sort (minimizes pairs shown); user swipes/taps to pick winner
3. **Define Properties** — open set of decision criteria, independent of options, extensible at any time
4. **Property Attribution** — for each option pair, user moves per-property sliders (how much better is A vs B on each property)
5. **Analysis** — correlate slider data against ranking to reveal dominant/surprising/inconsistent properties

### Data Model

```
DecisionSession
├── options: Set<Option>          // { id, label }
├── properties: Set<Property>     // { id, label }
├── comparisons: Map<PairKey, int>          // ranking decisions (-1, 0, 1), sparse
└── attributions: Map<PairKey, Map<PropertyId, float>>  // slider values, sparse
```

Both matrices are **sparse** — not every pair needs an explicit decision. Missing values are interpolated via transitivity or regression.

### Key Constraints
- **Single-threaded WASM only** — GitHub Pages cannot set `COOP`/`COEP` headers required for `SharedArrayBuffer` (multi-threaded WASM). Keep all computation on the main thread or use a service worker workaround.
- **No backend** — all analysis runs in C++ compiled to WASM in the browser.
- **Open sets** — adding an option or property never invalidates existing data; it adds unfilled cells to the sparse matrices.

### Analysis Engine
- Spearman/Kendall rank correlation between property slider sums and final ranking
- Identifies: dominant properties, surprising drivers (low declared weight but high actual correlation), inconsistencies (property ratings contradict ranking direction)
- Updates incrementally as more attributions are filled in
