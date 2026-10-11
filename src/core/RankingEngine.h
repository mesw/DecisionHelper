#pragma once
#include "Types.h"
#include <optional>

namespace RankingEngine {

// Returns the next pair to compare, or nullopt if ranking is complete.
std::optional<std::pair<OptionId, OptionId>> nextPair(const DecisionSession& s);

// Record a direct comparison result. winner/loser are option IDs.
void recordComparison(DecisionSession& s, OptionId winner, OptionId loser);
void recordTie(DecisionSession& s, OptionId a, OptionId b);

// Rebuild the transitive closure from scratch (call after load or any mutation).
void rebuildTransitiveClosure(DecisionSession& s);

// Assign integer rank positions from topological sort.
void assignRankPositions(DecisionSession& s);

// Progress: fraction of pairs resolved (answered or inferred).
float progress(const DecisionSession& s);

// Binary search insertion for a new (already-added) option.
// Returns pivot option ID to compare against, or nullopt if insertion is done.
struct InsertionState {
    OptionId newOptionId;
    int lo = 0;
    int hi = 0; // exclusive
};
std::optional<OptionId> insertionNextPivot(const DecisionSession& s, const InsertionState& st);
// Returns updated InsertionState; sets rankPosition when done (hi-lo == 0 after narrowing).
InsertionState insertionRecord(DecisionSession& s, InsertionState st, OptionId pivot, int result);

} // namespace RankingEngine
