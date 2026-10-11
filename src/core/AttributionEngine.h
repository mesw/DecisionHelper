#pragma once
#include "Types.h"
#include <vector>
#include <string>

namespace AttributionEngine {

struct AttributionCard {
    PairKey              pair;
    std::vector<PropertyId> properties; // up to kMaxSlidersPerCard
    std::string          rationale;
};

// Returns next card, or card with empty properties if done / no data yet.
AttributionCard nextCard(const DecisionSession& s);

// Record all slider values from a card submission.
void recordCard(DecisionSession& s, PairKey pair, const std::map<PropertyId, float>& values);

// True when all properties have k >= 3 OR answered >= min(C(n,2), 2*p)
bool isAtSweetSpot(const DecisionSession& s);

// Progress fraction toward sweet spot
float progress(const DecisionSession& s);

// For a newly-inserted option: select <= 3 pairs vs high-influence properties
std::vector<PairKey> pairsForNewOption(const DecisionSession& s, OptionId newId,
                                       const std::map<PropertyId, float>& propInfluence);

} // namespace AttributionEngine
