#pragma once
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <utility>

// ── ID types ─────────────────────────────────────────────────────────────────
using OptionId    = uint32_t;
using CriterionId = uint32_t;
using PairKey     = std::pair<uint32_t, uint32_t>; // always lo < hi

inline PairKey makePairKey(uint32_t a, uint32_t b) {
    return a < b ? PairKey{a, b} : PairKey{b, a};
}

// ── Core structs ─────────────────────────────────────────────────────────────
struct Option {
    OptionId    id = 0;
    std::string label;
    float       score = 0.f;       // 0.0 - 1.0 (or 0% - 100%)
    int         rankPosition = -1; // 0 = 1st, 1 = 2nd, etc.
};

struct Criterion {
    CriterionId id = 0;
    std::string label;
    float       weight = 0.f;      // 0.0 - 1.0 (sums to 1.0)
};

struct DecisionSession {
    uint32_t nextOptionId    = 1;
    uint32_t nextCriterionId = 1;
    std::string title;
    std::vector<Option>    options;
    std::vector<Criterion> criteria;

    // For each criterion ID: map of (OptionPair) -> (+1 = lo wins, -1 = hi wins, 0 = tie)
    std::map<CriterionId, std::map<PairKey, int>> optionComparisons;

    // Criteria comparisons: map of (CriterionPair) -> (+1 = lo wins, -1 = hi wins, 0 = tie)
    std::map<PairKey, int> criteriaComparisons;
};

// ── Derived result structs ───────────────────────────────────────────────────
struct OptionScoreBreakdown {
    CriterionId criterionId;
    std::string criterionLabel;
    float       criterionScore       = 0.f; // 0.0 - 1.0
    float       criterionWeight      = 0.f; // 0.0 - 1.0
    float       weightedContribution = 0.f; // weight * score
};

struct OptionResult {
    OptionId    optionId;
    std::string optionLabel;
    float       totalScore = 0.f; // 0.0 - 1.0
    int         rank       = 0;   // 1-based (1 = best)
    std::vector<OptionScoreBreakdown> breakdowns;
};
