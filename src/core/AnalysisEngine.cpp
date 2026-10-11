#include "AnalysisEngine.h"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <vector>
#include <numeric>

// ── Score aggregation (simple signed sum, §D MVP) ────────────────────────────

ScoreMatrix AnalysisEngine::computeScores(const DecisionSession& s) {
    ScoreMatrix scores;
    for (auto& opt : s.options)
        for (auto& prop : s.properties)
            scores[opt.id][prop.id] = 0.f;

    for (auto& [pk, propMap] : s.attributions.data) {
        OptionId lo = pk.first, hi = pk.second;
        for (auto& [pid, v] : propMap) {
            // positive v means lo better than hi
            scores[lo][pid] += v;
            scores[hi][pid] -= v;
        }
    }
    return scores;
}

// ── Rank a vector, return rank positions (1-based, avg for ties) ─────────────
static std::vector<float> rankVector(const std::vector<float>& vals) {
    size_t n = vals.size();
    std::vector<size_t> idx(n);
    std::iota(idx.begin(), idx.end(), 0);
    std::sort(idx.begin(), idx.end(), [&](size_t a, size_t b) { return vals[a] < vals[b]; });
    std::vector<float> ranks(n);
    for (size_t i = 0; i < n; ) {
        size_t j = i;
        while (j < n && vals[idx[j]] == vals[idx[i]]) ++j;
        float avgRank = (float)(i + j + 1) / 2.f; // 1-based average
        for (size_t k = i; k < j; ++k) ranks[idx[k]] = avgRank;
        i = j;
    }
    return ranks;
}

// ── Spearman ρ ───────────────────────────────────────────────────────────────
static float spearman(const std::vector<float>& scores, const std::vector<int>& rankPos) {
    size_t n = scores.size();
    if (n < 3) return 0.f;

    auto scoreRanks = rankVector(scores);
    // rankPos: 0 = best → convert to 1-based rank where 1 = best
    // We want rank 1 = position 0
    std::vector<float> gutRanks(n);
    for (size_t i = 0; i < n; ++i) gutRanks[i] = (float)(rankPos[i] + 1);

    float sumD2 = 0.f;
    for (size_t i = 0; i < n; ++i) {
        float d = scoreRanks[i] - gutRanks[i];
        sumD2 += d * d;
    }
    float rho = 1.f - 6.f * sumD2 / (float)(n * (n * n - 1));
    return std::max(-1.f, std::min(1.f, rho));
}

// ── Kendall τ ────────────────────────────────────────────────────────────────
static float kendall(const std::vector<float>& scores, const std::vector<int>& rankPos) {
    size_t n = scores.size();
    if (n < 2) return 0.f;
    int C = 0, D = 0;
    for (size_t i = 0; i < n; ++i)
        for (size_t j = i + 1; j < n; ++j) {
            float ds = scores[i] - scores[j];
            float dr = (float)(rankPos[i] - rankPos[j]);
            if (ds * dr < 0.f) ++D;
            else if (ds * dr > 0.f) ++C;
        }
    int total = (int)(n * (n - 1) / 2);
    return total > 0 ? (float)(C - D) / (float)total : 0.f;
}

// ── Compute correlations ─────────────────────────────────────────────────────

CorrelationMap AnalysisEngine::computeCorrelations(const DecisionSession& s,
                                                    const ScoreMatrix& scores) {
    CorrelationMap result;
    size_t n = s.options.size();
    if (n < 2) return result;

    for (auto& prop : s.properties) {
        std::vector<float> sc;
        std::vector<int>   rk;
        for (auto& opt : s.options) {
            auto sit = scores.find(opt.id);
            if (sit == scores.end()) continue;
            auto pit = sit->second.find(prop.id);
            if (pit == sit->second.end()) continue;
            sc.push_back(pit->second);
            rk.push_back(opt.rankPosition);
        }
        if (sc.size() < 2) { result[prop.id] = {}; continue; }

        float rho = spearman(sc, rk);
        float tau = kendall(sc, rk);
        float sigma_z = (sc.size() > 3) ? 1.f / std::sqrt((float)sc.size() - 3.f) : 1.f;

        result[prop.id] = { rho, tau, sigma_z, (int)sc.size() };
    }
    return result;
}

// ── Classification ───────────────────────────────────────────────────────────

ClassificationMap AnalysisEngine::classifyProperties(const DecisionSession& s,
                                                       const CorrelationMap& corr) {
    ClassificationMap cls;
    int p = (int)s.properties.size();

    for (auto& prop : s.properties) {
        auto it = corr.find(prop.id);
        if (it == corr.end()) { cls[prop.id] = PropertyRole::Noise; continue; }
        float rho = it->second.spearman_rho;

        bool high_priority  = (p > 0) && (prop.declaredPriority > 0) &&
                              (prop.declaredPriority <= (int)std::ceil(p / 2.0));
        bool strong_corr    = std::abs(rho) >= kStrongCorrelationThreshold;
        bool positive_corr  = rho > 0.f;
        bool inconsistent   = rho <= kInconsistencyThreshold;

        PropertyRole role;
        if (inconsistent) {
            role = PropertyRole::Inconsistency;
        } else if (strong_corr && positive_corr && !high_priority) {
            role = PropertyRole::HiddenDriver;
        } else if (strong_corr && positive_corr) {
            role = PropertyRole::DominantDriver;
        } else if (high_priority && !strong_corr) {
            role = PropertyRole::Overrated;
        } else {
            role = PropertyRole::Noise;
        }
        cls[prop.id] = role;
    }
    return cls;
}

// ── Values-based ranking ─────────────────────────────────────────────────────

ValuesRanking AnalysisEngine::valuesRanking(const DecisionSession& s, const ScoreMatrix& scores) {
    std::vector<std::pair<float, OptionId>> scored;
    for (auto& opt : s.options) {
        float vs = 0.f;
        for (auto& prop : s.properties) {
            if (prop.declaredPriority <= 0) continue;
            float w = 1.f / (float)prop.declaredPriority;
            auto sit = scores.find(opt.id);
            if (sit != scores.end()) {
                auto pit = sit->second.find(prop.id);
                if (pit != sit->second.end()) vs += w * pit->second;
            }
        }
        scored.push_back({vs, opt.id});
    }
    std::sort(scored.rbegin(), scored.rend()); // descending score
    ValuesRanking result;
    for (auto& [sc, id] : scored) result.push_back(id);
    return result;
}

// ── Insight narrative ─────────────────────────────────────────────────────────

std::string AnalysisEngine::insightNarrative(const DecisionSession& s,
                                              const CorrelationMap& corr,
                                              const ClassificationMap& cls,
                                              const ValuesRanking& valRanking) {
    if (s.options.empty()) return "Add options and complete ranking to see insights.";

    // Find gut #1
    std::string gutFirst;
    for (auto& opt : s.options)
        if (opt.rankPosition == 0) { gutFirst = opt.label; break; }

    // Find values #1
    std::string valFirst;
    if (!valRanking.empty())
        for (auto& opt : s.options)
            if (opt.id == valRanking[0]) { valFirst = opt.label; break; }

    // Find top declared priority property
    std::string topDeclared;
    float topDeclaredRho = 0.f;
    for (auto& prop : s.properties) {
        if (prop.declaredPriority == 1) {
            topDeclared = prop.label;
            auto it = corr.find(prop.id);
            if (it != corr.end()) topDeclaredRho = it->second.spearman_rho;
        }
    }

    // Find hidden driver
    std::string hiddenDriver;
    float hiddenRho = 0.f;
    int hiddenPriority = 0;
    for (auto& prop : s.properties) {
        auto cit = cls.find(prop.id);
        if (cit != cls.end() && cit->second == PropertyRole::HiddenDriver) {
            auto it = corr.find(prop.id);
            if (it != corr.end() && std::abs(it->second.spearman_rho) > std::abs(hiddenRho)) {
                hiddenDriver   = prop.label;
                hiddenRho      = it->second.spearman_rho;
                hiddenPriority = prop.declaredPriority;
            }
        }
    }

    std::ostringstream oss;
    oss << "Your gut chose <b>" << gutFirst << "</b>.";
    if (!valFirst.empty() && valFirst != gutFirst)
        oss << " By your stated priorities, <b>" << valFirst << "</b> would come first.";

    if (!topDeclared.empty()) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.2f", topDeclaredRho);
        std::string label = (std::abs(topDeclaredRho) < 0.3f) ? "weakly" :
                            (topDeclaredRho < 0.f) ? "negatively" : "moderately";
        oss << " Your stated top priority (<b>" << topDeclared << "</b>) " << label
            << " predicted your ranking (ρ = " << buf << ", n = " << (int)s.options.size() << ").";
    }

    if (!hiddenDriver.empty()) {
        char buf[64];
        std::snprintf(buf, sizeof(buf), "%.2f", hiddenRho);
        oss << " The real driver appears to be <b>" << hiddenDriver << "</b> (ρ = " << buf << ")"
            << " — which you ranked " << hiddenPriority << " in importance.";
    }

    return oss.str();
}
