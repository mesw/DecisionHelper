# DecisionHelper — Brainstorming & Specification

## Core Concept

A webapp that helps users make decisions and then *explains* those decisions — surfacing the true underlying reasoning, including factors the user may not have consciously recognized.

---

## How It Works (User Flow)

### Phase 1: Add Options
The user defines a set of options (e.g. job offers, apartments, project ideas). Options can be added at any time — the set is open-ended.

### Phase 2: Pairwise Ranking (Sorting Round)
The app presents pairs of options one at a time using a comparison-based sorting algorithm (e.g. merge sort, quicksort, or a more efficient algorithm like TimSort). The user decides which option they prefer — e.g. by swiping left/right or tapping. This produces a ranked ordering of options.

Not every pair needs to be compared explicitly. The sorting algorithm can infer transitive relationships (if A > B and B > C, then A > C). **Sparse decisions are supported** — missing comparisons are interpolated from the ranking structure.

**Output:** A ranked list of options reflecting the user's revealed preferences.

### Phase 3: Define Properties
After ranking, the user is prompted to define *properties* — the criteria or dimensions that might have driven their choices (e.g. salary, location, gut feel, growth potential). Properties can also be added at any time and are independent of the options set.

### Phase 4: Property Attribution Round
For each pair of options (or a sparse subset), the user sees a slider panel:

```
Option A  vs  Option B
[←——— Salary ———→]
[←——— Location ———→]
[←— Growth Potential —→]
```

Each slider represents how much better one option is vs the other on that property. The sliders are independent — this is not a forced allocation. The user rates each property dimension for each pair they're asked about.

Again, **sparse input is supported** — not every option pair × property combination needs to be filled in. Missing values are interpolated (e.g. via matrix completion, collaborative filtering, or simple transitivity).

### Phase 5: Analysis & Insights
The app compares the Phase 2 ranking against the Phase 4 property ratings and produces insights:

- **"Property D was the dominant factor in your decisions"** — high correlation between D ratings and final ranking
- **"You thought you preferred B for reason E, but the data suggests F was the real driver"** — low E correlation, high F correlation for B's wins
- **"Your decision was inconsistent on property G"** — G ratings contradict the ranking for certain pairs
- **"Options X and Y are nearly tied — here's what differentiates them"** — highlights the marginal factors

---

## Key Architectural Principles

### Open Sets
- Options and Properties are independent sets, each extensible at any time
- Adding a new option or property does not invalidate existing data — it just creates new unfilled cells in the comparison matrix
- The system tracks which pairs have been explicitly decided and which are inferred

### Sparse Decision Matrices
- Two matrices: **Ranking matrix** (option × option) and **Property matrix** (option × option × property)
- Explicit decisions populate cells; the rest are interpolated
- Interpolation strategy is pluggable (transitivity, regression, matrix factorization)

### Sorting Algorithm
- Use an adaptive comparison sort that minimizes the number of pairs shown to the user
- Track which comparisons were made explicitly vs. inferred
- Allow the user to revisit and override inferred comparisons

### Analysis Engine
- Correlate property sliders with final ranking using regression or rank correlation (Spearman/Kendall)
- Identify dominant properties, surprising drivers, and inconsistencies
- Results should update incrementally as more data is provided

---

## Data Model

```
Decision Session
├── options: Set<Option>          // { id, label, metadata }
├── properties: Set<Property>     // { id, label, description }
├── comparisons: Map<PairKey, -1|0|1>   // ranking decisions (sparse)
└── attributions: Map<PairKey, Map<PropertyId, float>>  // slider values (sparse)
```

---

## Tech Stack

- **Language:** C++17/20
- **UI Framework:** Qt6 with **QtQuick** (QML for declarative UI, QtQuick Controls 2 for components, QtQuick animations and gesture handling, Qt Charts for analysis visualizations)
- **Compilation target:** WebAssembly via Emscripten (`emcmake` + `emmake` + Qt for WebAssembly kit)
- **Hosting:** GitHub Pages (static deployment of the compiled WASM + HTML shell)
- **Persistence:** Browser `localStorage` via Qt's `QSettings` with a WebAssembly storage backend, or direct JS interop via `emscripten::val`
- **Analysis:** Pure C++ (Eigen or hand-rolled for rank correlation / linear regression) — runs entirely in-browser, no server needed
- **Build system:** CMake

### Deployment Pipeline
1. Build with Qt for WebAssembly (`wasm32-unknown-emscripten` target)
2. Output: `index.html`, `app.wasm`, `app.js`, `qtloader.js`
3. Push to `gh-pages` branch → served via GitHub Pages

### Key Considerations
- Qt6 WebAssembly builds are single-threaded by default; multi-threaded WASM requires `SharedArrayBuffer` (needs COOP/COEP headers, which GitHub Pages does not set — use single-threaded or a custom service worker workaround)
- Touch/swipe gestures handled via Qt Quick's `SwipeDelegate` or custom `MultiPointTouchArea`
- No backend required — fully client-side, offline-capable

---

## Open Questions / Future Directions

- How to handle **circular preferences** (A > B > C > A)? Flag and surface as insight.
- **Multi-user mode** — aggregate decisions from a group to find consensus or reveal disagreements
- **Weighted properties** — allow users to declare upfront how important each property is, then compare declared vs. revealed weights
- **Decision history** — track decisions over time to surface personal decision-making patterns
- **Export** — export the ranked list and analysis as a report (PDF/shareable link)
- **Onboarding** — guide first-time users through the flow with an example decision
