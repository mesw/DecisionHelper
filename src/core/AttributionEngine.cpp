#include "AttributionEngine.h"
#include <algorithm>
#include <cmath>
#include <sstream>

// Count how many pairs have been answered for property P
static int pairCount(const DecisionSession& s, PropertyId pid) {
    int k = 0;
    for (auto& [pair, propMap] : s.attributions.data)
        if (propMap.count(pid)) ++k;
    return k;
}

// Discriminating power: rank distance between two options
static int rankDist(const DecisionSession& s, OptionId a, OptionId b) {
    int ra = -1, rb = -1;
    for (auto& opt : s.options) {
        if (opt.id == a) ra = opt.rankPosition;
        if (opt.id == b) rb = opt.rankPosition;
    }
    if (ra < 0 || rb < 0) return 0;
    return std::abs(ra - rb);
}

AttributionEngine::AttributionCard AttributionEngine::nextCard(const DecisionSession& s) {
    AttributionCard card;
    if (s.options.size() < 2 || s.properties.empty()) return card;

    // Collect all pairs
    std::vector<std::pair<OptionId, OptionId>> pairs;
    for (size_t i = 0; i < s.options.size(); ++i)
        for (size_t j = i + 1; j < s.options.size(); ++j)
            pairs.push_back({s.options[i].id, s.options[j].id});

    // Find property with max gain (least answered)
    float maxGain = -1.f;
    for (auto& prop : s.properties) {
        int k = pairCount(s, prop.id);
        float gain = 1.f / std::sqrt((float)k + 1.f) - 1.f / std::sqrt((float)k + 2.f);
        if (gain > maxGain) maxGain = gain;
    }

    // Score each pair
    float bestScore = -1.f;
    for (auto& [a, b] : pairs) {
        PairKey pk = makePairKey(a, b);
        float disc = (float)rankDist(s, a, b);
        if (disc < 1.f) disc = 1.f;

        // Gain: max gain among properties not yet answered for this pair
        float pairGain = 0.f;
        for (auto& prop : s.properties) {
            bool answered = s.attributions.data.count(pk) &&
                            s.attributions.data.at(pk).count(prop.id);
            if (!answered) {
                int k = pairCount(s, prop.id);
                float g = 1.f / std::sqrt((float)k + 1.f) - 1.f / std::sqrt((float)k + 2.f);
                if (g > pairGain) pairGain = g;
            }
        }
        float score = disc * pairGain;
        if (score > bestScore) {
            bestScore = score;
            card.pair = pk;
        }
    }

    if (bestScore <= 0.f) return card; // nothing to ask

    // Select up to kMaxSlidersPerCard properties with lowest k for this pair
    std::vector<std::pair<int, PropertyId>> propsByK;
    for (auto& prop : s.properties) {
        bool answered = s.attributions.data.count(card.pair) &&
                        s.attributions.data.at(card.pair).count(prop.id);
        if (!answered) {
            propsByK.push_back({pairCount(s, prop.id), prop.id});
        }
    }
    std::sort(propsByK.begin(), propsByK.end());
    for (int i = 0; i < kMaxSlidersPerCard && i < (int)propsByK.size(); ++i)
        card.properties.push_back(propsByK[i].second);

    // Build rationale
    std::string la, lb;
    for (auto& opt : s.options) {
        if (opt.id == card.pair.first)  la = opt.label;
        if (opt.id == card.pair.second) lb = opt.label;
    }
    int dist = rankDist(s, card.pair.first, card.pair.second);
    std::ostringstream oss;
    oss << "Comparing " << la << " vs " << lb;
    if (dist > 1) oss << " — biggest ranking gap";
    oss << ". These properties need more data.";
    card.rationale = oss.str();

    return card;
}

void AttributionEngine::recordCard(DecisionSession& s, PairKey pair,
                                   const std::map<PropertyId, float>& values) {
    for (auto& [pid, v] : values)
        s.attributions.data[pair][pid] = v;
}

bool AttributionEngine::isAtSweetSpot(const DecisionSession& s) {
    size_t n = s.options.size();
    size_t p = s.properties.size();
    if (n < 2 || p == 0) return false;

    // Check all properties have k >= 3
    bool allThree = true;
    for (auto& prop : s.properties) {
        if (pairCount(s, prop.id) < 3) { allThree = false; break; }
    }
    if (allThree) return true;

    // Check total answered >= min(C(n,2), 2*p)
    size_t maxPairs = n * (n - 1) / 2;
    size_t target = std::min(maxPairs, 2 * p);
    size_t answered = s.attributions.data.size();
    return answered >= target;
}

float AttributionEngine::progress(const DecisionSession& s) {
    size_t n = s.options.size();
    size_t p = s.properties.size();
    if (n < 2 || p == 0) return 0.f;

    size_t maxPairs = n * (n - 1) / 2;
    size_t target = std::min(maxPairs, 2 * p);
    if (target == 0) return 1.f;

    size_t answered = s.attributions.data.size();
    return (float)std::min(answered, target) / (float)target;
}

std::vector<PairKey> AttributionEngine::pairsForNewOption(
    const DecisionSession& s, OptionId newId,
    const std::map<PropertyId, float>& propInfluence)
{
    // Select up to 3 pairs with the most influential properties
    std::vector<std::pair<float, OptionId>> byInfluence;
    for (auto& opt : s.options) {
        if (opt.id == newId) continue;
        float inf = 0.f;
        for (auto& [pid, rho] : propInfluence) inf += std::abs(rho);
        byInfluence.push_back({inf, opt.id});
    }
    std::sort(byInfluence.rbegin(), byInfluence.rend());

    std::vector<PairKey> result;
    for (int i = 0; i < 3 && i < (int)byInfluence.size(); ++i)
        result.push_back(makePairKey(newId, byInfluence[i].second));
    return result;
}
