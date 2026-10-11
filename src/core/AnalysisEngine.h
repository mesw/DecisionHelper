#pragma once
#include "Types.h"
#include <string>

namespace AnalysisEngine {

ScoreMatrix     computeScores(const DecisionSession& s);
CorrelationMap  computeCorrelations(const DecisionSession& s, const ScoreMatrix& scores);
ClassificationMap classifyProperties(const DecisionSession& s, const CorrelationMap& corr);
ValuesRanking   valuesRanking(const DecisionSession& s, const ScoreMatrix& scores);
std::string     insightNarrative(const DecisionSession& s,
                                  const CorrelationMap& corr,
                                  const ClassificationMap& cls,
                                  const ValuesRanking& valRanking);

} // namespace AnalysisEngine
