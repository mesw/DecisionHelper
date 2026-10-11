# ALGORITHMS.md — Simplified Algorithm Design

This document describes the simplified, transparent algorithms for **DecisionHelper**.

---

## 1. Design Principles

1. **Simplicity over Pseudo-Science:** No complex statistical regressions (Spearman $\rho$, Kendall $\tau$, Fisher $z$-transform) or matrix inversions. Real users make decisions with small sample sizes ($3 - 7$ options), where classical rank correlation produces noisy, confusing metrics.
2. **Discrete Micro-Decisions:** Replace continuous multi-attribute sliders ($-1.0$ to $+1.0$) with clean, discrete 3-way pairwise comparisons:
   - Option A is better
   - Both are equal
   - Option B is better
3. **Fair Criteria Weighting:** Replace artificial harmonic drag-and-drop decay ($1, 1/2, 1/3, \dots$) with pairwise criteria comparisons. This directly solves the problem where the #1 criterion was overweighted and multiple top-priority criteria could not share equal importance.
4. **Transparent Scoring:** The final score for each option is a straightforward weighted sum ($0\% - 100\%$), easily explainable to the user.

---

## 2. Option Scoring per Criterion (Micro-Decisions)

For each criterion $C_j$:
The user evaluates pairs of options $(O_a, O_b)$.

### Scoring Rule (Standard Pairwise / Copeland):
For each pair $(O_a, O_b)$:
- If $O_a$ is chosen: $O_a$ receives **1 point**, $O_b$ receives **0 points**.
- If $O_b$ is chosen: $O_b$ receives **1 point**, $O_a$ receives **0 points**.
- If **Equal** is chosen: $O_a$ receives **0.5 points**, $O_b$ receives **0.5 points**.

### Normalized Criterion Score:
For an option $O_i$ on criterion $C_j$:
$$\text{Points}(O_i, C_j) = \sum_{k \neq i} \text{Points earned against } O_k$$

The maximum possible points against $(n-1)$ other options is $(n-1)$.
$$\text{Score}(O_i, C_j) = \frac{\text{Points}(O_i, C_j)}{n - 1} \quad \in [0.0, 1.0]$$

*(Special case: If $n = 1$, score is $1.0$.)*

### Transitivity Shortcut (Optional):
If $O_a > O_b$ and $O_b > O_c$ on criterion $C_j$, we can automatically infer $O_a > O_c$ and skip asking the user, minimizing repetitive questions.
If any comparison is "Equal", transitivity does not infer strict inequality.

---

## 3. Criteria Weighting (Pairwise Comparison)

To determine how much each criterion matters, the user compares criteria pairwise: $(C_a, C_b)$.

### Comparison Rule:
- If $C_a$ is more important: $C_a$ receives **1 point**, $C_b$ receives **0 points**.
- If $C_b$ is more important: $C_b$ receives **1 point**, $C_a$ receives **0 points**.
- If **Equal** is chosen: both receive **0.5 points**.

### Normalized Criterion Weight:
Let $m$ be the number of criteria.
$$\text{Points}(C_j) = \sum_{k \neq j} \text{Points earned against } C_k$$

Total points distributed across all criteria is $\binom{m}{2}$:
$$W(C_j) = \frac{\text{Points}(C_j)}{\sum_{k=1}^m \text{Points}(C_k)}$$

*(If all comparisons are Equal or $m=1$, each criterion receives equal weight: $W(C_j) = \frac{1}{m}$.)*

### Why this is superior to Drag-Ranking:
- With drag-and-drop harmonic decay ($1/1, 1/2, 1/3$), Criterion #1 had $50\%$ of the weight compared to #2, and $4\times$ the weight of #4.
- In reality, users often have **multiple equally important top criteria** (e.g. *Price* and *Safety* are both top priority).
- With pairwise comparison, marking *Price = Safety* assigns both the exact same top score and weight.

---

## 4. Overall Option Score & Ranking

The overall decision score for option $O_i$ is the weighted linear combination:

$$\text{TotalScore}(O_i) = \sum_{j=1}^m W(C_j) \times \text{Score}(O_i, C_j)$$

### Presentation:
- Displayed as a clear percentage ($0\% - 100\%$).
- Options are ranked in descending order of $\text{TotalScore}$.
- A transparent breakdown shows the contribution of each criterion:
  $$\text{Contribution}(O_i, C_j) = W(C_j) \times \text{Score}(O_i, C_j)$$

---

## 5. UI Interaction Rules

1. **Equal Button Sizing:**
   In both option micro-decisions and criteria weighting, the user is presented with three options:
   ```
   [ Option A ]   [ Equally Good ]   [ Option B ]
   ```
   All three buttons/cards have **identical size**, **equal touch target areas**, and **balanced visual prominence**. The "Equal" option is never subordinated or styled as a tiny dismissive link.

2. **Progress Tracking:**
   Progress is simply:
   $$\text{Progress} = \frac{\text{Questions Answered}}{\text{Total Required Questions}}$$

---

## 6. Summary of Deprecated Algorithms

| Old Algorithm | Why It Was Removed | Replaced By |
|---|---|---|
| Spearman $\rho$ & Kendall $\tau$ | Inaccurate with tiny sample sizes ($n \le 7$), confusing to users | Direct weighted scoring |
| Fisher $z$-transform CI | Mathematical over-engineering for a personal decision tool | Removed |
| Latent Scores (L2 Regularized Least Squares) | Extreme complexity to invert sparse matrices in browser | Copeland point accumulation |
| Multi-Slider Attribution Cards | High cognitive fatigue moving 5 continuous sliders per pair | Discrete 3-way micro-decisions |
| Harmonic Drag-Ranking ($1/k$) | Overweighted #1 criterion, prohibited equal top priorities | Pairwise criteria weighting |
