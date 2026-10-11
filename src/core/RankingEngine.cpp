#include "RankingEngine.h"
#include <algorithm>
#include <vector>
#include <map>
#include <set>
#include <cassert>

// ── Internal helpers ─────────────────────────────────────────────────────────

// Is there any known result (direct or inferred) for this pair?
static bool isKnown(const DecisionSession& s, OptionId a, OptionId b) {
    PairKey key = makePairKey(a, b);
    if (s.comparisons.direct.count(key)) return true;
    if (s.comparisons.inferred.count(key)) return true;
    return false;
}

// ── Transitive closure ───────────────────────────────────────────────────────

// Build adjacency: winners → set of losers (direct beats only, non-tie)
static std::map<OptionId, std::set<OptionId>> buildBeatsGraph(const DecisionSession& s) {
    std::map<OptionId, std::set<OptionId>> g;
    for (auto& opt : s.options) g[opt.id]; // ensure all nodes present
    for (auto& [key, val] : s.comparisons.direct) {
        if (val == 0) continue; // tie
        OptionId lo = key.first, hi = key.second;
        OptionId winner = (val == 1) ? lo : hi;
        OptionId loser  = (val == 1) ? hi : lo;
        g[winner].insert(loser);
    }
    return g;
}

void RankingEngine::rebuildTransitiveClosure(DecisionSession& s) {
    s.comparisons.inferred.clear();
    auto g = buildBeatsGraph(s);

    // Floyd-Warshall reachability
    std::vector<OptionId> ids;
    for (auto& opt : s.options) ids.push_back(opt.id);
    size_t n = ids.size();

    std::vector<std::vector<bool>> reach(n, std::vector<bool>(n, false));
    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j)
            reach[i][j] = g[ids[i]].count(ids[j]) > 0;

    for (size_t k = 0; k < n; ++k)
        for (size_t i = 0; i < n; ++i)
            for (size_t j = 0; j < n; ++j)
                if (reach[i][k] && reach[k][j])
                    reach[i][j] = true;

    for (size_t i = 0; i < n; ++i)
        for (size_t j = 0; j < n; ++j) {
            if (i == j) continue;
            if (!reach[i][j]) continue;
            OptionId winner = ids[i], loser = ids[j];
            PairKey key = makePairKey(winner, loser);
            if (!s.comparisons.direct.count(key)) {
                // Encode direction: +1 = lo wins, -1 = hi wins
                int val = (winner == key.first) ? 1 : -1;
                s.comparisons.inferred[key] = val;
            }
        }
}

// ── Rank assignment ──────────────────────────────────────────────────────────

void RankingEngine::assignRankPositions(DecisionSession& s) {
    if (s.options.empty()) return;
    // Count how many options beat each option
    std::map<OptionId, int> beatenBy;
    for (auto& opt : s.options) beatenBy[opt.id] = 0;

    auto g = buildBeatsGraph(s);
    // Add inferred edges too
    for (auto& [pk, val] : s.comparisons.inferred) {
        OptionId winner = (val == 1) ? pk.first : pk.second;
        OptionId loser  = (val == 1) ? pk.second : pk.first;
        g[winner].insert(loser);
    }

    for (auto& [winner, losers] : g)
        for (auto loser : losers)
            beatenBy[loser]++;

    // Sort by beatenBy count
    std::vector<OptionId> sorted;
    for (auto& opt : s.options) sorted.push_back(opt.id);
    std::sort(sorted.begin(), sorted.end(), [&](OptionId a, OptionId b) {
        return beatenBy[a] < beatenBy[b];
    });

    for (size_t i = 0; i < sorted.size(); ++i)
        for (auto& opt : s.options)
            if (opt.id == sorted[i]) { opt.rankPosition = (int)i; break; }
}

// ── Merge-sort pair generation ───────────────────────────────────────────────
// We maintain virtual merge-sort state via the comparisons already recorded.
// Strategy: find any pair that is not yet known (direct or inferred).

std::optional<std::pair<OptionId, OptionId>> RankingEngine::nextPair(const DecisionSession& s) {
    // Prefer pairs that are adjacent in rank order (most informative for merge sort)
    std::vector<OptionId> ids;
    for (auto& opt : s.options) ids.push_back(opt.id);
    if (ids.size() < 2) return std::nullopt;

    // Try all pairs; prefer those closest together in inferred order
    // For simplicity: return first unknown pair (merge-sort adjacent pairs first)
    // Sort by rankPosition where available, else just id order
    std::sort(ids.begin(), ids.end(), [&](OptionId a, OptionId b) {
        int ra = -1, rb = -1;
        for (auto& opt : s.options) {
            if (opt.id == a) ra = opt.rankPosition;
            if (opt.id == b) rb = opt.rankPosition;
        }
        if (ra == -1 && rb == -1) return a < b;
        if (ra == -1) return false;
        if (rb == -1) return true;
        return ra < rb;
    });

    for (size_t i = 0; i + 1 < ids.size(); ++i) {
        OptionId a = ids[i], b = ids[i + 1];
        if (!isKnown(s, a, b)) return std::make_pair(a, b);
    }
    // Check all remaining pairs
    for (size_t i = 0; i < ids.size(); ++i)
        for (size_t j = i + 1; j < ids.size(); ++j)
            if (!isKnown(s, ids[i], ids[j]))
                return std::make_pair(ids[i], ids[j]);
    return std::nullopt;
}

// ── Record comparison ────────────────────────────────────────────────────────

void RankingEngine::recordComparison(DecisionSession& s, OptionId winner, OptionId loser) {
    PairKey key = makePairKey(winner, loser);
    int val = (winner == key.first) ? 1 : -1;
    s.comparisons.direct[key] = val;
    rebuildTransitiveClosure(s);
    assignRankPositions(s);
}

void RankingEngine::recordTie(DecisionSession& s, OptionId a, OptionId b) {
    PairKey key = makePairKey(a, b);
    s.comparisons.direct[key] = 0;
    rebuildTransitiveClosure(s);
    assignRankPositions(s);
}

// ── Progress ─────────────────────────────────────────────────────────────────

float RankingEngine::progress(const DecisionSession& s) {
    size_t n = s.options.size();
    if (n < 2) return 1.f;
    size_t total = n * (n - 1) / 2;
    size_t known = s.comparisons.direct.size() + s.comparisons.inferred.size();
    return (float)std::min(known, total) / (float)total;
}

// ── Binary search insertion ──────────────────────────────────────────────────

std::optional<OptionId> RankingEngine::insertionNextPivot(const DecisionSession& s, const InsertionState& st) {
    if (st.lo >= st.hi) return std::nullopt;
    // Find option with rankPosition == (lo + hi) / 2
    int mid = (st.lo + st.hi) / 2;
    for (auto& opt : s.options) {
        if (opt.id != st.newOptionId && opt.rankPosition == mid)
            return opt.id;
    }
    return std::nullopt;
}

RankingEngine::InsertionState RankingEngine::insertionRecord(
    DecisionSession& s, InsertionState st, OptionId pivot, int result)
{
    if (result > 0) {
        // new option beats pivot → insert before pivot
        st.hi = (st.lo + st.hi) / 2;
    } else if (result < 0) {
        // pivot beats new option → insert after pivot
        st.lo = (st.lo + st.hi) / 2 + 1;
    } else {
        // tie → insert at pivot position
        st.lo = st.hi = (st.lo + st.hi) / 2;
    }

    if (st.lo >= st.hi) {
        // Done: shift all options at rank >= lo up by 1, assign new option to lo
        for (auto& opt : s.options) {
            if (opt.id != st.newOptionId && opt.rankPosition >= st.lo)
                opt.rankPosition++;
        }
        for (auto& opt : s.options) {
            if (opt.id == st.newOptionId) { opt.rankPosition = st.lo; break; }
        }
    }
    return st;
}
