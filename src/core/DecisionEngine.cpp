#include "DecisionEngine.h"
#include <algorithm>
#include <cmath>

namespace DecisionEngine {

// ── Step 4: Option Micro-Decisions ───────────────────────────────────────────

std::vector<OptionMicroDecision> allOptionMicroDecisions(const DecisionSession& s) {
    std::vector<OptionMicroDecision> list;
    if (s.options.size() < 2 || s.criteria.empty()) return list;

    size_t n = s.options.size();
    size_t pairsPerCrit = n * (n - 1) / 2;
    int total = static_cast<int>(pairsPerCrit * s.criteria.size());
    int qIdx = 1;

    for (const auto& c : s.criteria) {
        const auto* compMap = [&]() -> const std::map<PairKey, int>* {
            auto it = s.optionComparisons.find(c.id);
            return (it != s.optionComparisons.end()) ? &(it->second) : nullptr;
        }();

        for (size_t i = 0; i < s.options.size(); ++i) {
            for (size_t j = i + 1; j < s.options.size(); ++j) {
                OptionMicroDecision dec;
                dec.criterionId     = c.id;
                dec.criterionLabel  = c.label;
                dec.optionAId       = s.options[i].id;
                dec.optionALabel    = s.options[i].label;
                dec.optionBId       = s.options[j].id;
                dec.optionBLabel    = s.options[j].label;
                dec.currentQuestion = qIdx++;
                dec.totalQuestions  = total;

                PairKey pk = makePairKey(dec.optionAId, dec.optionBId);
                if (compMap && compMap->find(pk) != compMap->end()) {
                    dec.hasAnswer = true;
                    int stored = compMap->at(pk);
                    if (stored == 0) {
                        dec.currentAnswer = 0;
                    } else if (dec.optionAId < dec.optionBId) {
                        dec.currentAnswer = (stored == 1) ? 1 : -1;
                    } else {
                        dec.currentAnswer = (stored == -1) ? 1 : -1;
                    }
                } else {
                    dec.hasAnswer     = false;
                    dec.currentAnswer = 999;
                }

                list.push_back(dec);
            }
        }
    }

    return list;
}

int optionMicroDecisionTotalCount(const DecisionSession& s) {
    size_t n = s.options.size();
    if (n < 2 || s.criteria.empty()) return 0;
    size_t pairsPerCriterion = n * (n - 1) / 2;
    return static_cast<int>(pairsPerCriterion * s.criteria.size());
}

int optionMicroDecisionAnsweredCount(const DecisionSession& s) {
    int count = 0;
    for (const auto& c : s.criteria) {
        auto it = s.optionComparisons.find(c.id);
        if (it != s.optionComparisons.end()) {
            count += static_cast<int>(it->second.size());
        }
    }
    return count;
}

float optionMicroDecisionProgress(const DecisionSession& s) {
    int total = optionMicroDecisionTotalCount(s);
    if (total == 0) return 1.0f;
    return std::min(1.0f, static_cast<float>(optionMicroDecisionAnsweredCount(s)) / static_cast<float>(total));
}

std::optional<OptionMicroDecision> nextOptionMicroDecision(const DecisionSession& s) {
    auto all = allOptionMicroDecisions(s);
    for (const auto& dec : all) {
        if (!dec.hasAnswer) return dec;
    }
    return std::nullopt;
}

void recordOptionComparison(DecisionSession& s, CriterionId criterionId,
                            OptionId optionA, OptionId optionB, int result) {
    PairKey pk = makePairKey(optionA, optionB);
    int val = 0;
    if (result == 0) {
        val = 0;
    } else if (optionA < optionB) {
        val = (result > 0) ? 1 : -1;
    } else {
        val = (result > 0) ? -1 : 1;
    }
    s.optionComparisons[criterionId][pk] = val;
}

void clearAllOptionComparisons(DecisionSession& s) {
    s.optionComparisons.clear();
}

// ── Step 5: Criteria Weighting ───────────────────────────────────────────────

std::vector<CriteriaWeightDecision> allCriteriaWeightDecisions(const DecisionSession& s) {
    std::vector<CriteriaWeightDecision> list;
    if (s.criteria.size() < 2) return list;

    size_t m = s.criteria.size();
    int total = static_cast<int>(m * (m - 1) / 2);
    int qIdx = 1;

    for (size_t i = 0; i < s.criteria.size(); ++i) {
        for (size_t j = i + 1; j < s.criteria.size(); ++j) {
            CriteriaWeightDecision dec;
            dec.criterionAId    = s.criteria[i].id;
            dec.criterionALabel = s.criteria[i].label;
            dec.criterionBId    = s.criteria[j].id;
            dec.criterionBLabel = s.criteria[j].label;
            dec.currentQuestion = qIdx++;
            dec.totalQuestions  = total;

            PairKey pk = makePairKey(dec.criterionAId, dec.criterionBId);
            auto it = s.criteriaComparisons.find(pk);
            if (it != s.criteriaComparisons.end()) {
                dec.hasAnswer = true;
                int stored = it->second;
                if (stored == 0) {
                    dec.currentAnswer = 0;
                } else if (dec.criterionAId < dec.criterionBId) {
                    dec.currentAnswer = (stored == 1) ? 1 : -1;
                } else {
                    dec.currentAnswer = (stored == -1) ? 1 : -1;
                }
            } else {
                dec.hasAnswer     = false;
                dec.currentAnswer = 999;
            }

            list.push_back(dec);
        }
    }

    return list;
}

int criteriaWeightingTotalCount(const DecisionSession& s) {
    size_t m = s.criteria.size();
    if (m < 2) return 0;
    return static_cast<int>(m * (m - 1) / 2);
}

int criteriaWeightingAnsweredCount(const DecisionSession& s) {
    return static_cast<int>(s.criteriaComparisons.size());
}

float criteriaWeightingProgress(const DecisionSession& s) {
    int total = criteriaWeightingTotalCount(s);
    if (total == 0) return 1.0f;
    return std::min(1.0f, static_cast<float>(criteriaWeightingAnsweredCount(s)) / static_cast<float>(total));
}

std::optional<CriteriaWeightDecision> nextCriteriaWeightDecision(const DecisionSession& s) {
    auto all = allCriteriaWeightDecisions(s);
    for (const auto& dec : all) {
        if (!dec.hasAnswer) return dec;
    }
    return std::nullopt;
}

void recordCriteriaComparison(DecisionSession& s, CriterionId criterionA,
                              CriterionId criterionB, int result) {
    PairKey pk = makePairKey(criterionA, criterionB);
    int val = 0;
    if (result == 0) {
        val = 0;
    } else if (criterionA < criterionB) {
        val = (result > 0) ? 1 : -1;
    } else {
        val = (result > 0) ? -1 : 1;
    }
    s.criteriaComparisons[pk] = val;
}

void clearAllCriteriaComparisons(DecisionSession& s) {
    s.criteriaComparisons.clear();
}

// ── Step 6: Calculations & Results ───────────────────────────────────────────

std::map<CriterionId, float> computeCriteriaWeights(const DecisionSession& s) {
    std::map<CriterionId, float> weights;
    if (s.criteria.empty()) return weights;

    if (s.criteria.size() == 1) {
        weights[s.criteria.front().id] = 1.0f;
        return weights;
    }

    std::map<CriterionId, float> points;
    for (const auto& c : s.criteria) {
        points[c.id] = 0.0f;
    }

    for (size_t i = 0; i < s.criteria.size(); ++i) {
        for (size_t j = i + 1; j < s.criteria.size(); ++j) {
            PairKey pk = makePairKey(s.criteria[i].id, s.criteria[j].id);
            auto it = s.criteriaComparisons.find(pk);
            if (it != s.criteriaComparisons.end()) {
                if (it->second == 1) {
                    points[s.criteria[i].id] += 1.0f;
                } else if (it->second == -1) {
                    points[s.criteria[j].id] += 1.0f;
                } else {
                    points[s.criteria[i].id] += 0.5f;
                    points[s.criteria[j].id] += 0.5f;
                }
            }
        }
    }

    float totalPoints = 0.0f;
    for (const auto& [id, p] : points) {
        totalPoints += p;
    }

    if (totalPoints > 0.001f) {
        for (const auto& [id, p] : points) {
            weights[id] = p / totalPoints;
        }
    } else {
        float equalWeight = 1.0f / static_cast<float>(s.criteria.size());
        for (const auto& c : s.criteria) {
            weights[c.id] = equalWeight;
        }
    }

    return weights;
}

std::vector<OptionResult> computeRankedResults(const DecisionSession& s) {
    std::vector<OptionResult> results;
    if (s.options.empty()) return results;

    auto weights = computeCriteriaWeights(s);
    float maxOpponents = static_cast<float>(s.options.size() > 1 ? s.options.size() - 1 : 1);

    for (const auto& opt : s.options) {
        OptionResult res;
        res.optionId    = opt.id;
        res.optionLabel = opt.label;
        res.totalScore  = 0.0f;

        for (const auto& c : s.criteria) {
            float points = 0.0f;

            if (s.options.size() > 1) {
                auto critIt = s.optionComparisons.find(c.id);
                if (critIt != s.optionComparisons.end()) {
                    for (const auto& other : s.options) {
                        if (other.id == opt.id) continue;
                        PairKey pk = makePairKey(opt.id, other.id);
                        auto compIt = critIt->second.find(pk);
                        if (compIt != critIt->second.end()) {
                            if (compIt->second == 0) {
                                points += 0.5f;
                            } else if (opt.id < other.id) {
                                if (compIt->second == 1) points += 1.0f;
                            } else {
                                if (compIt->second == -1) points += 1.0f;
                            }
                        }
                    }
                }
            }

            float criterionScore = (s.options.size() > 1) ? (points / maxOpponents) : 1.0f;
            float weight = (weights.find(c.id) != weights.end()) ? weights.at(c.id) : 0.0f;
            float contribution = weight * criterionScore;
            res.totalScore += contribution;

            OptionScoreBreakdown b;
            b.criterionId          = c.id;
            b.criterionLabel       = c.label;
            b.criterionScore       = criterionScore;
            b.criterionWeight      = weight;
            b.weightedContribution = contribution;
            res.breakdowns.push_back(b);
        }

        results.push_back(res);
    }

    // Sort descending by totalScore
    std::sort(results.begin(), results.end(), [](const OptionResult& a, const OptionResult& b) {
        return a.totalScore > b.totalScore;
    });

    // Assign rank positions
    for (size_t i = 0; i < results.size(); ++i) {
        results[i].rank = static_cast<int>(i + 1);
    }

    return results;
}

} // namespace DecisionEngine
