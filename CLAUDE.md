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

### User Flow (6 Phases)
1. **Add Options** — open set, extensible at any time
2. **Pairwise Ranking** — adaptive comparison sort (minimizes pairs shown via transitivity); user swipes/taps to pick winner; gut feel only, no analysis yet
3. **Define Properties** — open set of decision criteria, independent of options, extensible at any time
4. **Prioritise Properties** — user drag-ranks properties by declared importance (most → least); sets `declaredPriority`; enables declared-vs-actual contrast in analysis
5. **Property Attribution** — for each *selected* option pair (smart subset, not all n²), user moves per-property sliders (how much better is A vs B); pairs chosen by information gain
6. **Analysis** — correlate slider data against ranking; compare declared priority vs actual correlation; show values-based alternative ranking; surface hidden drivers and inconsistencies

**Post-analysis loops (no full restart needed):**
- **Add properties** → return to Phase 3, then skip Phase 4, go straight to Phase 5 with smart pair selection for new properties only
- **Add option** → binary search insertion (≤ ⌈log₂ n⌉ ranking questions), then smart attribution against high-influence properties only (≤ 3 pairs), then analysis updates inline

### Data Model

```
DecisionSession
├── options: Set<Option>          // { id, label, rankPosition }
├── properties: Set<Property>     // { id, label, declaredPriority }
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
- Compares `declaredPriority` against actual correlation to classify each property:
  - **Dominant driver** — high correlation, regardless of declared rank
  - **Surprising/hidden** — low declared priority but high actual correlation
  - **Overrated** — high declared priority but low/no actual correlation
  - **Inconsistency** — correlation direction contradicts ranking direction
- Computes a values-based alternative ranking from declared priorities; surfaces gap vs gut ranking
- Updates incrementally as more attributions are filled in
- Pair selection uses information gain to minimise questions while reaching a confidence threshold
