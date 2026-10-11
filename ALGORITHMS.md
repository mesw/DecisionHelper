# ALGORITHMS.md

Algorithm design for DecisionHelper. All decisions are resolved; open items are explicitly marked **[WATCH]** (monitor, not block).

---

## Overview of Algorithm Domains

| Domain | Phase | Core Question |
|---|---|---|
| [A. Adaptive Ranking](#a-adaptive-ranking) | 2 | Which pair to ask next? How to infer transitively? |
| [B. Binary Search Insertion](#b-binary-search-insertion) | Late add | Where does a new option fit? |
| [C. Attribution Pair Selection](#c-attribution-pair-selection-information-gain) | 5 | Which (pair, property) combos yield most signal? |
| [D. Score Aggregation](#d-score-aggregation) | 5→6 | How to turn sparse sliders into per-option property scores? |
| [E. Rank Correlation](#e-rank-correlation) | 6 | How strong is each property's tie to the gut ranking? |
| [F. Property Classification](#f-property-classification) | 6 | Hidden driver? Overrated? Inconsistency? |
| [G. Values-Based Alternative Ranking](#g-values-based-alternative-ranking) | 6 | Rank options by declared priorities instead of gut feel |
| [H. Confidence & Stopping Criterion](#h-confidence--stopping-criterion) | 5, 6 | When do we have enough data? |
| [I. Data Model](#i-data-model-algorithm-driven) | all | C++ structs driven by the algorithms |
| [J. WASM / Qt Feasibility](#j-wasm--qt-feasibility-assessment) | all | Is any algorithm a risk in single-threaded WASM? |
| [K. Visualisation](#k-visualisation--data-presentation) | 6 | Charts, animations, interaction model |

---

## A. Adaptive Ranking

### Problem

The user compares pairs until we have a full total order over n options. We want to ask the **minimum number of pairs** by exploiting transitivity: if A > B and B > C, we never ask A vs C.

### Data structure

A directed acyclic graph (DAG) of known orderings.

```
nodes: options
edges: A → B means "A beats B" (directly or by transitivity)
```

After each user answer we close transitively: for every new edge A → B, propagate all of A's ancestors as beating all of B's descendants.

**Transitive closure update (incremental):**
```
new_edge(A, B):
  for each X that can reach A:      // ancestors of A
    for each Y reachable from B:    // descendants of B
      add edge X → Y if not present
```

For n ≤ 20 this is O(n²) per step, trivially fast (<1 ms in WASM).

### Sorting strategy

**Merge sort with transitive short-circuit.** At each merge step, before presenting a comparison, check the DAG — if the answer is already derivable, skip and proceed. For n=5, worst case is 7 comparisons; transitivity typically reduces this to 4–5.

Tournament sort is equivalent and slightly more bracket-shaped, but we use a stream-of-pairs UI (see below) so the underlying merge-sort structure is invisible to the user.

### UI: stream of pairs

**Decided:** stream of pairs, one question at a time, with a progress bar.

Two option cards are presented simultaneously. Layout adapts to screen orientation:
- **Portrait:** cards stacked vertically (top / bottom), swipe up or down to choose.
- **Landscape:** cards side by side (left / right), swipe left or right to choose.

Swipe gesture: the chosen card animates to the centre of the screen; the rejected card slides off the opposite edge. Tap also works (tap the preferred option). Both interactions use the same decision handler — only the animation direction differs.

Progress bar below the cards. The bar has two markers:
- **Sweet spot** — a tick mark where the app has enough comparisons to establish a complete order (all C(n,2) pairs either answered or inferrable). This is the natural stopping point.
- **End** — all C(n,2) pairs answered. The user can continue past the sweet spot at any time; the bar fills further but does not wrap.

The progress percentage is: `inferred_or_answered / C(n,2)`.

### Tie handling

Ties (`comparisons[pair] = 0`) are valid but expensive: they break transitivity propagation (cannot infer A > C from A == B and B > C). If more than `⌊n/3⌋` ties accumulate, surface a gentle warning: "Lots of ties — you may want to revisit a few." No hard block.

---

## B. Binary Search Insertion

### Problem

A new option arrives after the ranking is complete. Find its position with ≤ ⌈log₂ n⌉ comparisons.

### Algorithm

The existing ranking is a sorted array `[rank_0, rank_1, ..., rank_{n-1}]` (index 0 = best).

```
insert(new_option, sorted_options):
  lo = 0, hi = n
  while lo < hi:
    mid = (lo + hi) / 2
    ask user: new_option vs sorted_options[mid]
    if new_option wins:
      hi = mid
    elif sorted_options[mid] wins:
      lo = mid + 1
    else:  // tie
      insert at mid, done
  insert at lo
```

Worst-case comparisons: ⌈log₂(n+1)⌉ — for n=5 that is 3 questions.

**Decided:** integer `rankPosition` renumbering. After insertion at index k, shift all positions ≥ k up by 1. For n ≤ 20 this is a trivial O(n) operation. Floating-point rank keys were considered and rejected: they add complexity and gain nothing at human-scale option counts.

UI uses the same stream-of-pairs card swipe as Phase 2. A sub-progress bar shows "Question 1 of ≤3" so the user knows it will be brief.

---

## C. Attribution Pair Selection (Information Gain)

### Problem

We have n options, p properties, and a sparse attribution matrix `A[pair][property] ∈ [−1, +1]`. The full matrix has `C(n,2) × p` cells. For n=5, p=5 that is 50 cells. We want to ask the user the **smallest subset** that still yields reliable property–ranking correlations.

### What "information" means here

For each property P, we want to estimate a **property score** for each option:

```
score(option_i, P) = aggregated attribution values involving option_i on P
```

Then we correlate `score(·, P)` against the gut ranking. We need enough data points per property to make the correlation estimate non-trivial.

### Selection algorithm: greedy variance reduction

**Decided:** greedy variance reduction scoring.

For each property P, maintain a running count of observed pairs `k(P)`. The Spearman CI width shrinks as `1/√(k−1)`. The expected reduction from one more answer is:

```
gain(P) = 1/√(k(P)) − 1/√(k(P)+1)
```

Pairs where the two options are far apart in the ranking provide more discriminating power:

```
discriminating_power(pair) = |rankPosition(A) − rankPosition(B)|
```

**Combined priority for selecting the next pair:**
```
priority(pair) = discriminating_power(pair) × max_P( gain(P) for properties not yet confident )
```

Select the pair with the highest priority. Within the selected pair, show the properties with the lowest `k(P)` first (most needed), up to the card limit (see below).

**Fallback (MVP):** if the greedy scorer proves complex to implement initially, use fixed heuristic: ask pairs `(rank_1, rank_n)`, `(rank_1, rank_⌊n/2⌋)`, `(rank_⌊n/2⌋, rank_n)` in order. Swap in greedy scoring once the pipeline is stable.

### Card layout: max 5 sliders per card

**Decided:** all properties for a selected pair are shown on one card (no card-flips mid-pair). If `p > 5`, the card shows the **5 properties with the lowest `k(P)`** (most data-hungry). Properties that are already well-characterised are deferred to later cards or omitted if the stopping criterion is reached.

`kMaxSlidersPerCard = 5` — named constant, easy to adjust.

Each card header explains why this pair was chosen:
> "Comparing **Pizza** vs **Salad** — biggest ranking gap. These properties need more data."

This text is derived directly from the pair-selection algorithm state; no extra bookkeeping needed.

---

## D. Score Aggregation

### Problem

Turn sparse attribution sliders into a single per-option score for each property.

### Attribution values

Each slider for pair (A, B) on property P produces `v ∈ [−1, +1]`:
- `+1` = A is much better than B on P
- `0` = equal
- `−1` = B is much better than A on P

### Approach: two-stage (MVP → accurate)

**Stage 1 — Simple signed sum (MVP):**

```
score(option_i, P) = Σ_j v(i, j, P) − Σ_j v(j, i, P)
```

Fast, no matrix inversion, handles sparse data naturally. Biased when options appear in different numbers of pairs, but acceptable for the expected data density.

Preliminary results with sparse data are valid with this formula — as more pairs are answered, scores refine. Analysis updates live.

**Stage 2 — Least squares (upgrade path):**

Each option has a latent score `s_i(P)`. The attribution value is:

```
v(A, B, P) ≈ s_A(P) − s_B(P)
```

Given k observed pairs for property P, solve:

```
minimize ||M·s − v||²
```

where M is the `k × n` incidence matrix (+1 at col A, −1 at col B per row). For n ≤ 20, k ≤ 40 — Gaussian elimination in <1 ms. Add L2 regulariser `λ||s||²` when `k < n−1` to prevent under-determination; fix `λ = 0.01` (named constant `kScoreRegulariser`).

**Decided:** raw scores are kept as-is throughout the analysis pipeline. Normalisation to [0, 1] is applied **only at the visualisation layer** (for display and cross-property comparison in charts). The correlation computation (Section E) operates on raw scores — this preserves the ability to show preliminary results at any sparsity level without a normalisation artefact from a single outlier dominating a small sample.

---

## E. Rank Correlation

### Problem

Given `score(option_i, P)` and `rankPosition(option_i)`, compute how strongly property P predicts the ranking.

### Spearman ρ (primary)

Convert both score vector and rank vector to rank-order, then:

```
ρ = 1 − (6 · Σd²) / (n · (n²−1))
```

where `d_i = rank(score_i) − rankPosition_i`.

Confidence interval via Fisher z-transform:
```
z  = atanh(ρ)
σ_z = 1 / √(n − 3)
CI  = (tanh(z − 1.96·σ_z),  tanh(z + 1.96·σ_z))
```

For n=5, σ_z ≈ 0.71 — CI is intentionally wide. This is expected and is communicated to the user.

### Kendall τ (secondary)

Count concordant (C) minus discordant (D) pairs, normalised:

```
τ = (C − D) / C(n, 2)
```

O(n²) computation, fine for n ≤ 20. More robust to ties than Spearman.

**Show both:** Spearman ρ is the headline number. Kendall τ appears in the expanded detail panel. They tell the same story for small n.

### Display

**Decided:** scientific look and feel. Show:
- Qualitative label (prominent)
- Spearman ρ value
- n (sample size)
- Standard deviation σ_z

Example display in the UI:
```
Cheese Content
━━━━━━━━━━━━━━━━━━━━
Strong driver      ρ = 0.97
                   n = 5,  σ = 0.71
```

For comparison, Kendall τ = 0.90 would appear in the tooltip/detail sheet.

### Qualitative bands

| \|ρ\| range | Label |
|---|---|
| 0.8 – 1.0 | Strong driver |
| 0.5 – 0.8 | Moderate influence |
| 0.2 – 0.5 | Weak signal |
| 0.0 – 0.2 | Negligible |

Negative ρ → **Inconsistency** (property favours lower-ranked options).

---

## F. Property Classification

Based on `ρ(P)` and `declaredPriority(P)`:

```cpp
constexpr float kStrongCorrelationThreshold = 0.6f;   // [WATCH] revisit after user testing
constexpr float kInconsistencyThreshold     = -0.4f;

bool high_priority     = priority_rank(P) <= ceil(p / 2.0);
bool strong_corr       = std::abs(rho(P)) >= kStrongCorrelationThreshold;
bool positive_corr     = rho(P) > 0;
bool inconsistent      = rho(P) <= kInconsistencyThreshold;
```

| Classification | Condition |
|---|---|
| **Dominant driver** | `strong_corr AND positive_corr` |
| **Hidden driver** | `strong_corr AND positive_corr AND NOT high_priority` |
| **Overrated** | `high_priority AND NOT strong_corr` |
| **Inconsistency** | `inconsistent` |
| **Noise** | everything else |

Hidden driver is a sub-type of dominant driver. In the UI, hidden drivers get a special callout banner: *"You didn't rank this highly — but it's driving everything."*

**[WATCH]** `kStrongCorrelationThreshold = 0.6f` is a fixed constant for now. Monitor whether it produces false positives or misses obvious drivers in real sessions. It may need to be adaptive to n later.

---

## G. Values-Based Alternative Ranking

### Problem

Rank options as if the user's declared priorities were the true weights.

### Algorithm

**Decided:** harmonic weighting on drag order (no numeric fine-tuning). The drag-order UX for declaring property priority mirrors the same interaction pattern used when gut-ranking options in Phase 2 — consistent mental model throughout the app.

```
for each option O:
  values_score(O) = Σ_P  (1 / priority_rank(P))  ×  score(O, P)
```

`1/rank` gives harmonic decay: rank 1 → weight 1.0, rank 2 → 0.5, rank 3 → 0.33, etc.

Sort options by `values_score` descending → values-based ranking.

This score is computed from raw (unnormalised) property scores. The resulting ranking is an ordinal output, so scale differences between properties do not affect correctness. For the side-by-side visualisation (K2), we display rank positions, not the raw `values_score`.

---

## H. Confidence & Stopping Criterion

### Problem

When does the user have enough attribution data to see meaningful patterns?

### Metric

Each property P has k observed attribution pairs. CI width:

```
CI_width(P) ≈ 2 × 1.96 × 1/√(k − 1)
```

For n=5, C(5,2)=10 pairs max — a truly tight CI is unachievable. This is acceptable: **the goal is surfacing patterns for reflection, not statistical significance.**

### Practical stopping rule

The "sweet spot" is reached when **all properties have k ≥ 3 data points**, OR the user has answered `min(C(n,2), 2p)` questions, whichever comes first.

For n=5, p=5: sweet spot after min(10, 10) = 10 questions, typically 4–6 in practice due to pair reuse across properties.

### Progress bar design

**Decided:** soft stop with a sweet-spot marker on the progress bar.

```
[████████████|░░░░░░░░░░░░░░░░░░░░]
              ↑
         Sweet spot
    "Good to analyse"
```

- Left of the sweet-spot tick: required minimum.
- Right of the tick: additional precision. User can continue freely.
- When the bar reaches the sweet spot, a non-blocking nudge appears: *"You've answered enough to see patterns. Keep going for more precision, or head to Analysis."*
- No hard stop. The Analysis button is available at any time (even before the sweet spot), with a confidence indicator showing how sparse the data is.

---

## I. Data Model (Algorithm-Driven)

The data structures are dictated by the algorithms above, not the UI.

```cpp
using OptionId   = uint32_t;
using PropertyId = uint32_t;
using PairKey    = std::pair<OptionId, OptionId>; // always lo < hi, enforced at construction

struct Option {
    OptionId    id;
    std::string label;
    int         rankPosition;  // 0 = best; -1 = unranked
};

struct Property {
    PropertyId  id;
    std::string label;
    int         declaredPriority;  // 1 = most important; 0 = unset
};

struct Comparisons {
    std::map<PairKey, int>  direct;    // user answers: +1 / 0 / -1
    std::set<PairKey>       inferred;  // transitive closure cache (never persisted)
};

struct Attributions {
    // v in [-1, +1]; positive = first option better on this property
    std::map<PairKey, std::map<PropertyId, float>> data;
};

struct DecisionSession {
    std::vector<Option>    options;
    std::vector<Property>  properties;
    Comparisons            comparisons;
    Attributions           attributions;
};
```

Note: `std::string` in core structs; convert to `QString` only at the QML boundary via `QString::fromStdString`.

### Derived structures (computed, never stored)

```cpp
// Raw score matrix: score[optionId][propertyId]
using ScoreMatrix = std::map<OptionId, std::map<PropertyId, float>>;

// Correlation results per property
struct PropertyCorrelation {
    float spearman_rho;
    float kendall_tau;
    float std_dev;      // σ_z from Fisher z-transform
    int   n;            // number of options
};
using CorrelationMap = std::map<PropertyId, PropertyCorrelation>;

// Classification
enum class PropertyRole { DominantDriver, HiddenDriver, Overrated, Inconsistency, Noise };
using ClassificationMap = std::map<PropertyId, PropertyRole>;

// Values-based ranking: ordered list of option IDs
using ValuesRanking = std::vector<OptionId>;
```

### Named constants

```cpp
constexpr float kStrongCorrelationThreshold = 0.6f;   // [WATCH]
constexpr float kInconsistencyThreshold     = -0.4f;
constexpr float kScoreRegulariser           = 0.01f;
constexpr int   kMaxSlidersPerCard          = 5;
```

### Serialisation (localStorage)

`DecisionSession` serialises to JSON. Only `direct` comparisons and `attributions.data` are persisted — the transitive closure `inferred` is always recomputed on load. Estimated max JSON size for n=10, p=10: ~5 KB. localStorage limit ~5 MB: no concern.

---

## J. WASM / Qt Feasibility Assessment

| Algorithm | Complexity | WASM risk | Notes |
|---|---|---|---|
| Transitive closure (A) | O(n³) worst | None | n ≤ 20; <1 ms |
| Binary search insertion (B) | O(log n) | None | Trivial |
| Pair selection scoring (C) | O(n² × p) per step | None | ≤ 2000 ops |
| Simple sum score (D MVP) | O(k × p) | None | Trivial |
| Least squares score (D full) | O(k × n²) | None | Matrix ≤ 20×20 |
| Spearman / Kendall (E) | O(n²) | None | n ≤ 20 |
| Classification (F) | O(p) | None | Trivial |
| Values ranking (G) | O(n × p) | None | Trivial |
| JSON serialisation | O(data size) | None | <5 KB |
| Qt Charts (K1, K3) | render only | Low | Qt Charts works in WASM with Qt 6.x; verify with Emscripten during build setup |

**Conclusion:** all algorithms are O(n²) or better. Single-threaded WASM is not a constraint. No SIMD, no threads, no BLAS needed — pure C++17 STL is sufficient.

**Qt Charts + WASM:** Qt Charts renders via OpenGL ES / WebGL in the browser. This works with Qt 6.x + Emscripten, but requires `QT += charts` in CMake and the appropriate Emscripten GL flags. Verify early in the build setup.

---

## K. Visualisation & Data Presentation

All decisions resolved. Animations are used everywhere the data changes — Qt Quick `Behavior on` and `NumberAnimation` are the primary tools.

### K1. Declared vs Actual scatter plot

**The core insight chart.** Interactive with Qt Charts `QScatterSeries`.

- X-axis: declared priority rank (1 = most important, p = least)
- Y-axis: actual Spearman ρ (−1 to +1)
- Each point: one property, coloured by role

```
ρ (actual)
 1.0 |                        • Cheese  ← hidden driver (amber)
     |
 0.5 |    • Tastiness  (green)
     |
 0.0 |----+----+----+----+----→ declared priority rank
     |         • Healthiness  ← overrated (red)
-0.5 |
```

Role colours:
- Hidden driver → amber
- Dominant driver (expected) → green
- Overrated → red
- Inconsistency → crimson
- Noise → grey

**Interaction:** tap a point → bottom sheet opens with:
- Property name
- ρ value, n, σ (scientific display)
- Kendall τ (secondary)
- One-line plain-language interpretation

Qt Charts tooltips or a custom QML `Popup` overlay both work. Custom popup gives more control over layout.

Quadrant lines (dashed) at ρ=0 and at the median declared rank divide the chart into four zones with faint background tints and small labels ("expected", "hidden", "overrated", "noise").

### K2. Ranking comparison (animated side-by-side)

Two vertical columns: **Gut Ranking** | **Values Ranking**. Options shown as labelled cards. SVG/Canvas lines connect each option across columns (alluvial style), with line thickness proportional to the magnitude of position change.

Animated: when the values ranking is first computed (or updated after new pairs), option cards animate to their new positions using Qt Quick `Behavior on y: NumberAnimation { duration: 400; easing.type: Easing.InOutCubic }`. Connector lines animate simultaneously.

Implementation: two `ListView`s with a `Canvas` overlay. The canvas redraws on `positionChanged` signal from the list items.

### K3. Per-property bar chart

Horizontal bar chart, one bar per property.
- Bar length = |ρ|
- Direction: positive ρ → right, negative ρ (inconsistency) → left
- Colour = role (same scheme as K1)
- Each bar labelled with property name and ρ value

**Qt Charts `QHorizontalBarSeries`** — standard component. Bars animate to their final values on first render: `QAbstractSeries::opacity` animated from 0 + Qt Charts built-in animation (`chart.animationOptions = ChartView.SeriesAnimations`).

### K4. Pair question progress grid

A small `n × p` grid showing which attribution cells have been answered, are inferred, or are outstanding. The currently-selected pair row is highlighted.

- Answered: filled (role colour)
- Inferred (from least squares extrapolation): lighter fill, dashed border
- Outstanding: empty
- Selected next: pulsing highlight

**Decided:** collapsed by default, expandable. Lives in a `DisclosurePanel` QML component below the current question card. Label: "Show data coverage ▾". Collapsed height = 0; expanded height = grid natural size. Animated with `Behavior on height`.

### K5. Insight narrative card

Plain-language summary at the top of Phase 6. Generated in C++ from `ClassificationMap`, exposed as `Q_PROPERTY QString insightNarrative`.

Template:
```
"Your gut chose [#1 option].
Your stated top priority ([property]) actually [weakly / negatively] predicted
your ranking (ρ = X, n = 5).
The real driver appears to be [hidden driver] (ρ = Y) —
which you ranked [Nth] in importance."
```

Rendered as a styled QML `Text` with inline bold and colour via HTML subset (`<b>`, `<font color="...">`). No chart — text only.

---

## Decision Log

All decisions resolved as of 2026-03-15.

| # | Question | Decision |
|---|---|---|
| A1 | Ranking UI: bracket vs stream of pairs? | **Stream of pairs.** Two cards, swipe to choose. Portrait: top/bottom swipe. Landscape: left/right swipe. Progress bar with sweet-spot marker. |
| B1 | rankPosition: float keys vs integer renumber? | **Integer renumber.** Trivial at n ≤ 20; clean for display. |
| C1 | Attribution cards: all properties vs one-at-a-time? | **All properties per pair on one card, max 5 sliders.** If p > 5, show the 5 lowest-coverage properties. `kMaxSlidersPerCard = 5`. |
| D1 | Normalise scores before correlation? | **Keep raw scores in the pipeline.** Normalise to [0,1] at the visualisation layer only. Supports live preliminary results at any sparsity. |
| E1 | Show numeric ρ or qualitative label only? | **Both.** Scientific display: qualitative label + ρ + n + σ. |
| F1 | Strong correlation threshold configurable? | **Fixed at `kStrongCorrelationThreshold = 0.6f`.** [WATCH] |
| G1 | Weighting: harmonic vs numeric weights? | **Harmonic decay on drag order only.** Same drag-order UX as option ranking — consistent mental model. |
| H1 | Hard stop vs soft stop? | **Soft stop.** Sweet-spot tick on progress bar; non-blocking nudge; Analysis always accessible. |
| K1 | Scatter plot: interactive? | **Interactive.** Qt Charts `QScatterSeries`, tap → bottom sheet detail. |
| K2 | Ranking comparison: animated? | **Animated.** Qt Quick `Behavior on y`. |
| K4 | Progress grid: always visible? | **Collapsed by default**, expandable via `DisclosurePanel`. |
