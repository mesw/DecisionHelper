# ALGORITHMS.md

Algorithm design for DecisionHelper. Each section presents the problem, viable options with tradeoffs, and a **recommended approach** — but marks decisions that need sign-off with **[DECISION NEEDED]**.

---

## Overview of Algorithm Domains

| Domain | Phase | Core Question |
|---|---|---|
| [A] Adaptive Ranking](#a-adaptive-ranking) | 2 | Which pair to ask next? How to infer transitively? |
| [B] Binary Search Insertion](#b-binary-search-insertion) | Late add | Where does a new option fit? |
| [C] Attribution Pair Selection](#c-attribution-pair-selection-information-gain) | 5 | Which (pair, property) combos yield most signal? |
| [D] Score Aggregation](#d-score-aggregation) | 5→6 | How to turn sparse sliders into per-option property scores? |
| [E] Rank Correlation](#e-rank-correlation) | 6 | How strong is each property's tie to the gut ranking? |
| [F] Property Classification](#f-property-classification) | 6 | Hidden driver? Overrated? Inconsistency? |
| [G] Values-Based Alternative Ranking](#g-values-based-alternative-ranking) | 6 | Rank options by declared priorities instead of gut feel |
| [H] Confidence & Stopping Criterion](#h-confidence--stopping-criterion) | 5, 6 | When do we have enough data? |

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

After each user answer, we close transitively: for every new edge A → B, propagate all of A's ancestors as beating all of B's descendants.

**Transitive closure update (incremental):**
```
new_edge(A, B):
  for each X that can reach A:      // ancestors of A
    for each Y reachable from B:    // descendants of B
      add edge X → Y if not present
```

For n ≤ 10 this is trivially fast (at most 100 edges).

### Sorting strategy options

| Option | Worst-case comparisons | Notes |
|---|---|---|
| **Merge sort** | ⌈n log₂ n⌉ | ~12 for n=5; predictable; natural fit for DAG inference |
| Insertion sort | O(n²) worst, O(n) best | Good if list is nearly sorted; less predictable |
| Quick sort | O(n log n) avg, O(n²) worst | Pivot choice matters; harder to exploit transitivity |
| Tournament sort | ⌈n log₂ n⌉ | Same as merge; slightly more natural for "bracket" UI |

**Recommendation:** Merge sort with transitive short-circuit. At each merge step, before asking a comparison, check the DAG — if the answer is already known, skip and proceed. For n=5, worst case is 7 comparisons but transitivity often reduces this to 4–5.

**[DECISION NEEDED]:** Do you want the ranking UI to feel like a "bracket" (tournament-style, visualizes progress as a bracket tree) or a "stream of pairs" (user just answers one question at a time, progress shown as a bar)? The algorithm is the same either way but shapes the QML.

### Tie handling

Ties are valid (`comparisons[pair] = 0`). They break transitivity propagation (can't infer A > C from A == B and B > C). Tie-heavy rankings will require more questions. We can warn the user if too many ties accumulate.

---

## B. Binary Search Insertion

### Problem

A new option arrives after the ranking is complete. We need to find its position with ≤ ⌈log₂ n⌉ comparisons.

### Algorithm

The existing ranking is a sorted array `[rank_1, rank_2, ..., rank_n]` (position 0 = best).

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

**This is deterministic and has no variants worth discussing.** The only design question:

**[DECISION NEEDED]:** After insertion, do we renumber all `rankPosition` values (a simple index shift), or do we use a floating-point rank key that allows gapless insertion without renumbering? Floating-point keys (e.g. rank as a `double` between neighbours) are simpler to implement but harder to display. Integer renumbering is cleaner for display. **Recommendation: integer renumbering** — for n ≤ 20 it is trivial.

---

## C. Attribution Pair Selection (Information Gain)

### Problem

We have n options, p properties, and need to populate the attribution matrix `A[pair][property]` ∈ [−1, +1]. The full matrix has `C(n,2) × p` cells. For n=5, p=5 that's 50 cells. We want to ask the user the **smallest subset** that still lets us compute reliable property–ranking correlations.

This is the mathematically richest problem in the app.

### What "information" means here

For each property P, we want to estimate a **property score** for each option:

```
score(option_i, P) = sum of attribution values involving option_i on property P
```

Then we correlate `score(·, P)` against the gut ranking. We need enough attribution data points per property to make the correlation estimate reliable.

### Option 1: Greedy variance reduction (recommended)

**Model:** For each property P, maintain a running estimate of `corr(P)` and its confidence interval (CI), based on data collected so far.

At each step, for each unasked (pair, property) cell, estimate the **expected reduction in CI width** if we were to ask it. Ask the cell with the highest expected reduction.

**How to estimate CI width reduction:**
- Spearman correlation confidence interval via Fisher z-transform: `σ ≈ 1/√(n−3)` for n data points
- Each new (pair, property) answer adds one data point for property P's score estimation
- CI width shrinks proportional to `1/√(data_points)`
- **Expected reduction ∝ 1/√(k+1) − 1/√k** where k = current data points for property P

In practice: ask pairs that cover properties with the fewest data points, prioritising pairs where the two options are **far apart in the ranking** (high discriminating power).

**Discriminating power of a pair:** Pairs where rank difference is large yield more signal per question. For ranking [1,2,3,4,5], pair (1,5) is maximally informative; pair (2,3) is minimally informative.

**Combined score for selecting next (pair, property):**
```
priority(pair, property) = rank_distance(pair) × (1 / sqrt(data_count(property) + 1))
```

Ask the (pair, property) with the highest priority. Stop when all properties have CI width below threshold (see [H](#h-confidence--stopping-criterion)).

### Option 2: D-optimal design (overkill)

In experimental design, D-optimal chooses observations to minimise the determinant of the covariance matrix of estimates. This gives provably minimal questions but requires matrix inversion at each step. For n ≤ 10 and p ≤ 10, the matrix is tiny (10×10 max), so it's computationally feasible in WASM — but it's harder to implement and explain.

**Verdict: too complex for the gain. Stick with Option 1.**

### Option 3: Fixed heuristic (simple but blind)

Always ask the top-3 rank-separated pairs (e.g., rank 1 vs rank n, rank 1 vs rank ⌊n/2⌋, rank ⌊n/2⌋ vs rank n). Fast to implement, predictable UI, but ignores which properties are already well-characterised.

**Verdict: use as a fallback/MVP if Option 1 proves complex during implementation.**

### UI implication

Each question card shows *why that pair was chosen*:
> "Comparing Pizza vs Salad — the biggest gap in your ranking, and Cheese Content has the least data so far."

This is derivable directly from the selection algorithm's intermediate state, so no extra bookkeeping is needed.

**[DECISION NEEDED]:** Do we present all properties for a chosen pair at once (one card, multiple sliders), or one (pair, property) at a time? Presenting all properties per pair is fewer card flips; one at a time allows aborting early if the property is already confident. **Recommendation: all properties per pair, one card per pair** — simpler UX, and stopping mid-card is confusing.

---

## D. Score Aggregation

### Problem

Turn sparse attribution sliders into a single per-option score for each property, so we can run correlation.

### Attribution values

Each slider for pair (A, B) on property P produces a value `v ∈ [−1, +1]`:
- `+1` = A is much better than B on P
- `0` = equal
- `−1` = B is much better than A on P

This is a **pairwise comparison** on property P, forming a second DAG (or score function estimation problem).

### Option 1: Least squares score estimation (recommended)

Model: each option has a latent score `s_i(P)`. The attribution value for pair (A, B) is:

```
v(A, B, P) ≈ s_A(P) − s_B(P)
```

Given k observed attributions for property P, solve for the score vector `s` via least squares:

```
minimize ||M·s − v||²
```

where M is the `k × n` incidence matrix (row i has +1 at column A, −1 at column B for the i-th observed pair).

This is a standard overdetermined linear system. For n ≤ 10 and k ≤ 20, solving it is trivial even without BLAS — Gaussian elimination or Gram-Schmidt by hand.

**Handles missing data naturally:** unobserved pairs just don't appear in M. The system is under-determined if k < n−1, but we can add a regulariser (`||s||² penalty`) or simply fix `s_1 = 0` (anchor the top-ranked option).

### Option 2: Simple sum of observed attributions

```
score(option_i, P) = sum(v(i, j, P) for all j) − sum(v(j, i, P) for all j)
```

Counts each observed pair's attribution, ignoring unobserved ones. Fast and simple, but biased if some options appear in fewer pairs than others.

**Verdict: Option 2 as MVP, Option 1 for accuracy.** Start with Option 2; swap in least squares once the basic pipeline works.

### Option 3: Bradley-Terry model

A probabilistic model where `P(A beats B on P) = s_A / (s_A + s_B)`. Fit via maximum likelihood (iterative). More principled but complex; overkill for this problem size.

**Verdict: defer indefinitely.**

**[DECISION NEEDED]:** Do we normalise scores to [0, 1] per property before correlation, or keep raw values? Normalisation makes cross-property comparisons fair in the visualisation. **Recommendation: normalise** — min-max per property after aggregation.

---

## E. Rank Correlation

### Problem

Given `score(option_i, P)` and `rankPosition(option_i)`, compute how strongly property P's scores predict the ranking.

### Option 1: Spearman ρ (recommended)

Convert both score and rank to rank-order, then apply Pearson correlation on the ranks:

```
ρ = 1 − (6 · Σd²) / (n · (n²−1))
```

where `d_i = rank(score_i) − rank(rankPosition_i)`.

- Ranges from −1 to +1
- Robust to outliers (ranks, not raw values)
- Fisher z-transform gives a confidence interval: `z = atanh(ρ)`, `σ_z = 1/√(n−3)`
- For n=5: σ_z ≈ 0.71, so CI is wide — this is expected and should be communicated to the user

### Option 2: Kendall τ

Count concordant minus discordant pairs, normalised by C(n,2). More robust to ties than Spearman. Slightly harder to compute (O(n²) naive, O(n log n) with merge sort). For n ≤ 10, O(n²) is fine.

**Verdict: implement both, display Spearman primary, offer Kendall as detail.** They tell the same story for small n.

### Significance note

For n=5, any correlation is statistically noisy. We should not show a p-value or call things "significant" — instead use qualitative bands:

| |ρ| range | Label |
|---|---|
| 0.8 – 1.0 | Strong driver |
| 0.5 – 0.8 | Moderate influence |
| 0.2 – 0.5 | Weak signal |
| 0.0 – 0.2 | Negligible |

Negative ρ → **Inconsistency** (property favours options ranked lower).

**[DECISION NEEDED]:** Should we show the numeric ρ to the user, or only the qualitative label? Showing numbers feels scientific but may be misleading given small n. **Recommendation: show both** — qualitative label prominently, ρ in a detail/tooltip.

---

## F. Property Classification

Based on `ρ(P)` (actual correlation) and `declaredPriority(P)` (user's stated rank):

```
priority_rank(P) = position of P in user's declared priority list (1 = most important)
high_priority     = priority_rank(P) ≤ ceil(p/2)
strong_correlation = |ρ(P)| ≥ 0.6
```

| Classification | Condition |
|---|---|
| **Dominant driver** | `strong_correlation AND ρ > 0` |
| **Hidden driver** | `strong_correlation AND ρ > 0 AND NOT high_priority` |
| **Overrated** | `high_priority AND NOT strong_correlation` |
| **Inconsistency** | `ρ < −0.4` (actively working against ranking) |
| **Noise** | everything else |

A property can be both "dominant driver" and "hidden driver" — the hidden flag is a sub-type. In the UI, hidden drivers get a special callout ("you didn't think this mattered, but it's driving everything").

**[DECISION NEEDED]:** The `0.6` threshold for "strong" is a judgment call. Do you want this configurable (expert mode), fixed, or adaptive to n (stricter threshold for larger n)? **Recommendation: fixed at 0.6 for now**, revisit after user testing. Document it as a named constant `kStrongCorrelationThreshold`.

---

## G. Values-Based Alternative Ranking

### Problem

Rank the options as if the user's **declared priorities** were the true weights — show what the "rational" choice would be given their stated values.

### Algorithm

```
for each option O:
  values_score(O) = Σ_P  (1 / priority_rank(P))  ×  score(O, P)
```

Using `1/rank` as the weight gives a Borda-like score: #1 priority gets weight 1.0, #2 gets 0.5, #3 gets 0.33, etc. (harmonic decay).

Sort options by `values_score` descending → values-based ranking.

### Alternative weighting schemes

| Scheme | Formula | Notes |
|---|---|---|
| **Harmonic (recommended)** | `1 / rank` | Natural decay, matches intuition |
| Linear | `(p − rank + 1) / p` | Shallower decay |
| Exponential | `0.5^(rank−1)` | Steep decay — #1 dominates |
| Raw declared importance | user-entered 1–10 score | More expressive but adds UI complexity |

**[DECISION NEEDED]:** Harmonic is the simplest to explain. But if the user explicitly enters numeric importance weights (e.g., "healthiness is 3× more important than price"), we get a richer model. Do you want a **two-tier priority system** — drag ordering (Phase 4) plus optional numeric fine-tuning — or just drag order? **Recommendation: just drag order for now.** The gap between gut and values ranking is already compelling without fine-tuning.

---

## H. Confidence & Stopping Criterion

### Problem

When do we have enough attribution data to trust the analysis? When can we stop asking the user questions in Phase 5?

### Metric: effective sample size per property

Each property P has k data points (observed attribution pairs). The Spearman CI shrinks as `1/√(k−1)`. We want the CI width ≤ some threshold τ.

```
CI_width(P) ≈ 2 × z_α × 1/√(k−1)
```

For α=0.20 (80% CI), z_α ≈ 1.28:
- k=3 → CI width ≈ 1.81 (too wide)
- k=4 → CI width ≈ 1.48
- k=5 → CI width ≈ 1.28

For n=5, we can never get a tight CI — the ranking only has C(5,2)=10 pairs max. Accept this: **the goal is not statistical rigor, it is surfacing patterns for reflection.**

### Practical stopping rule

Stop Phase 5 when **all properties have ≥ 3 data points** (pairs answered), OR the user has answered `min(C(n,2), 2p)` questions, whichever comes first.

For n=5, p=5: stop after min(10, 10) = 10 questions max, but typically 4–6 due to pair re-use across properties.

**[DECISION NEEDED]:** Should the user be able to **continue past the stopping point** (answer more pairs for higher confidence), or is the stop hard? **Recommendation: soft stop** — show "You've answered enough to see patterns. Keep going for more precision?" with a Continue button.

---

## I. Data Model (Algorithm-Driven)

The data structures are dictated by the algorithms above, not by the UI. Here is the concrete C++ model:

```cpp
using OptionId   = uint32_t;
using PropertyId = uint32_t;
using PairKey    = std::pair<OptionId, OptionId>; // always lo < hi

struct Option {
    OptionId  id;
    QString   label;
    int       rankPosition;  // 0 = best; -1 = unranked
};

struct Property {
    PropertyId id;
    QString    label;
    int        declaredPriority;  // 1 = most important; 0 = unset
};

struct Comparison {
    // +1 = first beats second, -1 = second beats first, 0 = tie
    std::map<PairKey, int> data;

    // Transitive closure cache: derived edges (not user-entered)
    std::set<PairKey> inferred;
};

struct Attribution {
    // value in [-1, +1]: positive = first option better on this property
    std::map<PairKey, std::map<PropertyId, float>> data;
};

struct DecisionSession {
    std::vector<Option>    options;
    std::vector<Property>  properties;
    Comparison             comparisons;
    Attribution            attributions;
};
```

### Derived structures (computed, not stored)

These are recomputed on demand — never persisted, never cached beyond a single analysis call:

```cpp
// Score matrix: score[optionId][propertyId]  (from least-squares or sum)
using ScoreMatrix = std::map<OptionId, std::map<PropertyId, float>>;

// Correlation vector: rho[propertyId]
using CorrelationMap = std::map<PropertyId, float>;

// Classification: classification[propertyId]
enum class PropertyRole { DominantDriver, HiddenDriver, Overrated, Inconsistency, Noise };
using ClassificationMap = std::map<PropertyId, PropertyRole>;
```

### Serialisation (localStorage)

The `DecisionSession` serialises to JSON via a hand-written encoder (no Qt JSON overhead worries at this data size). Keys are uint32_t IDs; values are primitives. Estimated max JSON size for n=10, p=10: ~5 KB. localStorage limit is 5–10 MB — no concern.

---

## J. WASM / Qt Feasibility Assessment

| Algorithm | Compute complexity | WASM risk | Notes |
|---|---|---|---|
| Transitive closure (A) | O(n³) worst | None | n ≤ 20; <1 ms |
| Binary search insertion (B) | O(log n) | None | Trivial |
| Pair selection scoring (C) | O(n² × p) per step | None | ≤ 2000 ops |
| Least squares (D) | O(k × n²) Gaussian elim | None | Matrix ≤ 20×20 |
| Spearman / Kendall (E) | O(n²) | None | n ≤ 20 |
| Classification (F) | O(p) | None | Trivial |
| Values ranking (G) | O(n × p) | None | Trivial |
| JSON serialisation | O(data size) | None | <5 KB |

**Conclusion: all algorithms are O(n²) or better on data sizes of n, p ≤ 20. Single-threaded WASM is not a constraint here. No SIMD, no threads, no BLAS needed. Pure C++17 with STL is sufficient.**

The only Qt-specific concern: `QString` in the data model. Keep data structs pure C++ (`std::string`), convert to `QString` only at the QML boundary.

---

## K. Visualisation & Data Presentation

These are informed by the algorithm outputs. Sorted by priority.

### K1. Declared vs Actual scatter plot (CRITICAL)

**The core insight chart.** X-axis = declared priority rank (1 = most important). Y-axis = actual correlation ρ. Each point is a property, labelled.

```
ρ (actual)
1.0 |                        • Cheese ← hidden driver
    |
0.5 |    • Tastiness
    |
0.0 |----+----+----+----+----→ declared rank (1=most important)
    |         • Healthiness ← overrated
-0.5|
    |
```

Quadrants:
- Top-left (high priority, high ρ) → expected drivers (boring, validates user)
- Top-right (low priority, high ρ) → **hidden drivers** (the reveal)
- Bottom-left (high priority, low ρ) → **overrated** (the confrontation)
- Bottom-right (low priority, low ρ) → noise (safe to ignore)

**Qt Charts `QScatterSeries`** — straightforward. Point colour encodes role (hidden = amber, overrated = red, driver = green, noise = grey).

**[DECISION NEEDED]:** Interactive (tap a point to see explanation) or static? **Recommendation: interactive** — tap opens a bottom sheet with the property name, its ρ, its declared rank, and a one-line plain-language interpretation.

### K2. Ranking comparison (side-by-side)

Two vertical lists: **Gut Ranking** | **Values-Based Ranking**. Options are shown as cards; lines connect the same option across columns to show movement (like an alluvial / Sankey strip).

Not available off-the-shelf in Qt Charts. Implement as a custom QML `Canvas` or as two `ListView`s with a `Canvas` overlay drawing the connecting lines.

**[DECISION NEEDED]:** Animated (options slide to new positions) or static diff? Animation is more legible for small n. **Recommendation: animated** — Qt Quick `Behavior on y` + a state machine.

### K3. Per-property bar chart

Horizontal bar chart: one bar per property, bar length = |ρ|, colour = role (driver/hidden/overrated/noise/inconsistency). Negative ρ bars extend left. Shows the full property landscape at a glance.

**Qt Charts `QHorizontalBarSeries`** — standard.

### K4. Pair question progress indicator

During Phase 5: show a small grid of n×p cells. Answered cells are filled; selected-next is highlighted; inferred cells (from transitivity/regression) shown in a lighter shade. Communicates sparsity and progress visually.

Custom QML `GridView` or `Canvas`. No Qt Charts needed.

**[DECISION NEEDED]:** Is this grid too "data-nerdy" for a consumer-facing app? Could be opt-in (expandable "details" panel). **Recommendation: show it collapsed by default**, expandable for curious users.

### K5. Insight narrative card

Plain-language summary at the top of Phase 6. Generated from the classification map:

```
Template:
"Your gut chose [#1 option]. Your stated top value ([#1 declared property])
actually [weak/negative] correlates with your ranking (ρ = X).
The real driver appears to be [hidden driver property] (ρ = Y),
which you ranked [Nth] in importance."
```

No chart — just a text `Label` in QML with inline bold/colour markup. Generated entirely from the `ClassificationMap` in C++, exposed to QML as a `Q_PROPERTY QString insightNarrative`.

---

## Open Decisions Summary

| # | Question | Recommendation | Status |
|---|---|---|---|
| A1 | Ranking UI feel: bracket vs stream of pairs? | Stream of pairs | **[NEEDS SIGN-OFF]** |
| B1 | rankPosition: float keys vs integer renumber? | Integer renumber | **[NEEDS SIGN-OFF]** |
| C1 | Attribution cards: all-properties-per-pair vs one-at-a-time? | All properties per pair | **[NEEDS SIGN-OFF]** |
| D1 | Normalise scores to [0,1] before correlation? | Yes | **[NEEDS SIGN-OFF]** |
| E1 | Show numeric ρ to user or only qualitative label? | Both (label prominent, ρ in tooltip) | **[NEEDS SIGN-OFF]** |
| F1 | Strong correlation threshold: fixed 0.6 vs configurable? | Fixed `kStrongCorrelationThreshold = 0.6f` | **[NEEDS SIGN-OFF]** |
| G1 | Weighting: harmonic decay vs user-entered numeric weights? | Harmonic (drag order only) | **[NEEDS SIGN-OFF]** |
| H1 | Stopping criterion: hard stop vs soft stop with Continue? | Soft stop | **[NEEDS SIGN-OFF]** |
| K1 | Scatter plot: interactive (tap for detail) vs static? | Interactive | **[NEEDS SIGN-OFF]** |
| K2 | Ranking comparison: animated vs static? | Animated | **[NEEDS SIGN-OFF]** |
| K4 | Pair progress grid: always visible vs collapsed/opt-in? | Collapsed by default | **[NEEDS SIGN-OFF]** |
