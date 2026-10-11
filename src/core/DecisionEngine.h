#pragma once
#include "Types.h"
#include <optional>
#include <vector>
#include <string>
#include <map>

namespace DecisionEngine {

// ── Option Micro-Decisions (Step 4) ──────────────────────────────────────────
struct OptionMicroDecision {
    CriterionId criterionId;
    std::string criterionLabel;
    OptionId    optionAId;
    std::string optionALabel;
    OptionId    optionBId;
    std::string optionBLabel;
    int         currentQuestion = 0; // 1-based index
    int         totalQuestions  = 0;
    int         currentAnswer   = 999; // +1 (A), -1 (B), 0 (Equal), 999 (unanswered)
    bool        hasAnswer       = false;
};

// Returns all micro-decision questions with their answered state and choices.
std::vector<OptionMicroDecision> allOptionMicroDecisions(const DecisionSession& s);

// Returns next unanswered micro-decision, or std::nullopt if all completed.
std::optional<OptionMicroDecision> nextOptionMicroDecision(const DecisionSession& s);

// Record comparison: result > 0 (A wins), result < 0 (B wins), result == 0 (Equal)
void recordOptionComparison(DecisionSession& s, CriterionId criterionId,
                            OptionId optionA, OptionId optionB, int result);

void clearAllOptionComparisons(DecisionSession& s);

float optionMicroDecisionProgress(const DecisionSession& s);
int   optionMicroDecisionTotalCount(const DecisionSession& s);
int   optionMicroDecisionAnsweredCount(const DecisionSession& s);

// ── Criteria Weighting (Step 5) ──────────────────────────────────────────────
struct CriteriaWeightDecision {
    CriterionId criterionAId;
    std::string criterionALabel;
    CriterionId criterionBId;
    std::string criterionBLabel;
    int         currentQuestion = 0; // 1-based index
    int         totalQuestions  = 0;
    int         currentAnswer   = 999; // +1 (A), -1 (B), 0 (Equal), 999 (unanswered)
    bool        hasAnswer       = false;
};

// Returns all criteria weight questions with their answered state and choices.
std::vector<CriteriaWeightDecision> allCriteriaWeightDecisions(const DecisionSession& s);

// Returns next unanswered criteria comparison, or std::nullopt if done or criteria <= 1.
std::optional<CriteriaWeightDecision> nextCriteriaWeightDecision(const DecisionSession& s);

// Record criteria comparison: result > 0 (A wins), result < 0 (B wins), result == 0 (Equal)
void recordCriteriaComparison(DecisionSession& s, CriterionId criterionA,
                              CriterionId criterionB, int result);

void clearAllCriteriaComparisons(DecisionSession& s);

float criteriaWeightingProgress(const DecisionSession& s);
int   criteriaWeightingTotalCount(const DecisionSession& s);
int   criteriaWeightingAnsweredCount(const DecisionSession& s);

// ── Results & Calculations (Step 6) ──────────────────────────────────────────
std::map<CriterionId, float> computeCriteriaWeights(const DecisionSession& s);
std::vector<OptionResult>    computeRankedResults(const DecisionSession& s);

} // namespace DecisionEngine
