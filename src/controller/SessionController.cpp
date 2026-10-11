#include "SessionController.h"
#include "../core/Session.h"
#include "../core/Storage.h"
#include <algorithm>
#include <cmath>

static OptionId toId(const QString& s) { return s.toUInt(); }
static QString  fromId(uint32_t id)    { return QString::number(id); }

SessionController::SessionController(QObject* parent) : QObject(parent) {}

bool SessionController::hasSavedSession() const {
    return Storage::exists(kStorageKey);
}

void SessionController::autosave() {
    Storage::save(kStorageKey, Session::toJson(m_session));
    emit hasSavedSessionChanged();
}

void SessionController::refreshPhaseState() {
    m_resultsDirty = true;
    emit microDecisionChanged();
    emit criteriaWeightChanged();
    emit resultsChanged();
}

// ── Navigation ───────────────────────────────────────────────────────────────

void SessionController::goToPhase(int phase) {
    if (phase < 1 || phase > 6) return;
    if (m_phase == phase) return;
    m_phase = phase;

    if (m_phase == 4) {
        auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
        int firstUnanswered = -1;
        for (size_t i = 0; i < questions.size(); ++i) {
            if (!questions[i].hasAnswer) {
                firstUnanswered = static_cast<int>(i);
                break;
            }
        }
        if (firstUnanswered >= 0) {
            m_microDecisionIndex = firstUnanswered;
            m_showMicroDecisionComplete = false;
        } else {
            // All answered: default to question 0 for adjusting, show question cards
            m_microDecisionIndex = 0;
            m_showMicroDecisionComplete = false;
        }
    } else if (m_phase == 5) {
        auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
        int firstUnanswered = -1;
        for (size_t i = 0; i < questions.size(); ++i) {
            if (!questions[i].hasAnswer) {
                firstUnanswered = static_cast<int>(i);
                break;
            }
        }
        if (firstUnanswered >= 0) {
            m_criteriaWeightIndex = firstUnanswered;
            m_showCriteriaWeightComplete = false;
        } else {
            m_criteriaWeightIndex = 0;
            m_showCriteriaWeightComplete = false;
        }
    }

    refreshPhaseState();
    emit currentPhaseChanged();
}

void SessionController::nextPhase() {
    if (m_phase < 6) goToPhase(m_phase + 1);
}

void SessionController::previousPhase() {
    if (m_phase > 1) goToPhase(m_phase - 1);
}

void SessionController::adjustMicroDecisions() {
    m_phase = 4;
    m_microDecisionIndex = 0;
    m_showMicroDecisionComplete = false;
    refreshPhaseState();
    emit currentPhaseChanged();
}

void SessionController::adjustCriteriaWeights() {
    m_phase = 5;
    m_criteriaWeightIndex = 0;
    m_showCriteriaWeightComplete = false;
    refreshPhaseState();
    emit currentPhaseChanged();
}

// ── Session Management ───────────────────────────────────────────────────────

void SessionController::newSession() {
    m_session = DecisionSession();
    m_phase = 1;
    m_microDecisionIndex = 0;
    m_showMicroDecisionComplete = false;
    m_criteriaWeightIndex = 0;
    m_showCriteriaWeightComplete = false;
    refreshPhaseState();
    emit currentPhaseChanged();
    emit decisionTitleChanged();
    emit optionsChanged();
    emit criteriaChanged();
    autosave();
}

void SessionController::loadSession() {
    if (!Storage::exists(kStorageKey)) return;
    std::string json = Storage::load(kStorageKey);
    m_session = Session::fromJson(json);

    // Pick sensible starting phase based on what's entered
    if (m_session.title.empty()) {
        m_phase = 1;
    } else if (m_session.options.size() < 2) {
        m_phase = 2;
    } else if (m_session.criteria.empty()) {
        m_phase = 3;
    } else if (!DecisionEngine::nextOptionMicroDecision(m_session).has_value() &&
               !DecisionEngine::nextCriteriaWeightDecision(m_session).has_value()) {
        m_phase = 6;
    } else if (!DecisionEngine::nextOptionMicroDecision(m_session).has_value()) {
        m_phase = 5;
    } else {
        m_phase = 4;
    }

    refreshPhaseState();
    emit currentPhaseChanged();
    emit decisionTitleChanged();
    emit optionsChanged();
    emit criteriaChanged();
}

void SessionController::saveSession() {
    autosave();
}

// ── Phase 1: Decision Name ───────────────────────────────────────────────────

QString SessionController::decisionTitle() const {
    return QString::fromStdString(m_session.title);
}

void SessionController::setDecisionTitle(const QString& title) {
    std::string t = title.trimmed().toStdString();
    if (m_session.title != t) {
        m_session.title = t;
        emit decisionTitleChanged();
        autosave();
    }
}

bool SessionController::canProceedToPhase2() const {
    return !m_session.title.empty();
}

// ── Phase 2: Options ─────────────────────────────────────────────────────────

QVariantList SessionController::options() const {
    QVariantList list;
    for (const auto& opt : m_session.options) {
        QVariantMap m;
        m["id"]           = fromId(opt.id);
        m["label"]        = QString::fromStdString(opt.label);
        m["rankPosition"] = opt.rankPosition;
        m["score"]        = opt.score;
        list.append(m);
    }
    return list;
}

void SessionController::addOption(const QString& label) {
    QString trimmed = label.trimmed();
    if (trimmed.isEmpty()) return;

    Option opt;
    opt.id    = m_session.nextOptionId++;
    opt.label = trimmed.toStdString();
    m_session.options.push_back(opt);

    refreshPhaseState();
    emit optionsChanged();
    autosave();
}

void SessionController::removeOption(const QString& id) {
    OptionId oid = toId(id);

    // Remove option
    auto& opts = m_session.options;
    opts.erase(std::remove_if(opts.begin(), opts.end(),
        [oid](const Option& o) { return o.id == oid; }), opts.end());

    // Clean up comparisons involving this option
    for (auto& [critId, pairMap] : m_session.optionComparisons) {
        for (auto it = pairMap.begin(); it != pairMap.end(); ) {
            if (it->first.first == oid || it->first.second == oid) {
                it = pairMap.erase(it);
            } else {
                ++it;
            }
        }
    }

    refreshPhaseState();
    emit optionsChanged();
    autosave();
}

// ── Phase 3: Criteria ────────────────────────────────────────────────────────

QVariantList SessionController::criteria() const {
    QVariantList list;
    for (const auto& c : m_session.criteria) {
        QVariantMap m;
        m["id"]     = fromId(c.id);
        m["label"]  = QString::fromStdString(c.label);
        m["weight"] = c.weight;
        list.append(m);
    }
    return list;
}

void SessionController::addCriterion(const QString& label) {
    QString trimmed = label.trimmed();
    if (trimmed.isEmpty()) return;

    Criterion c;
    c.id    = m_session.nextCriterionId++;
    c.label = trimmed.toStdString();
    m_session.criteria.push_back(c);

    refreshPhaseState();
    emit criteriaChanged();
    autosave();
}

void SessionController::removeCriterion(const QString& id) {
    CriterionId cid = toId(id);

    // Remove criterion
    auto& crits = m_session.criteria;
    crits.erase(std::remove_if(crits.begin(), crits.end(),
        [cid](const Criterion& c) { return c.id == cid; }), crits.end());

    // Remove option comparisons for this criterion
    m_session.optionComparisons.erase(cid);

    // Remove criteria comparisons involving this criterion
    for (auto it = m_session.criteriaComparisons.begin(); it != m_session.criteriaComparisons.end(); ) {
        if (it->first.first == cid || it->first.second == cid) {
            it = m_session.criteriaComparisons.erase(it);
        } else {
            ++it;
        }
    }

    refreshPhaseState();
    emit criteriaChanged();
    autosave();
}

// ── Phase 4: Option Micro-Decisions ──────────────────────────────────────────

QString SessionController::currentCriterionLabel() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return QString::fromStdString(questions[idx].criterionLabel);
}

QString SessionController::optionPairLeftId() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return fromId(questions[idx].optionAId);
}

QString SessionController::optionPairLeftLabel() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return QString::fromStdString(questions[idx].optionALabel);
}

QString SessionController::optionPairRightId() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return fromId(questions[idx].optionBId);
}

QString SessionController::optionPairRightLabel() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return QString::fromStdString(questions[idx].optionBLabel);
}

QString SessionController::microDecisionQuestion() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return QStringLiteral("No comparisons available");
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return QString("Comparison %1 of %2").arg(idx + 1).arg(questions.size());
}

float SessionController::microDecisionProgress() const {
    return DecisionEngine::optionMicroDecisionProgress(m_session);
}

bool SessionController::microDecisionsComplete() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return true;
    for (const auto& q : questions) {
        if (!q.hasAnswer) return false;
    }
    return true;
}

int SessionController::currentOptionAnswer() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return 999;
    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    return questions[idx].currentAnswer;
}

bool SessionController::hasPreviousMicroDecision() const {
    return m_microDecisionIndex > 0;
}

bool SessionController::hasNextMicroDecision() const {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    return m_microDecisionIndex + 1 < static_cast<int>(questions.size());
}

void SessionController::previousMicroDecision() {
    if (m_microDecisionIndex > 0) {
        m_microDecisionIndex--;
        m_showMicroDecisionComplete = false;
        emit microDecisionChanged();
    }
}

void SessionController::nextMicroDecision() {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (m_microDecisionIndex + 1 < static_cast<int>(questions.size())) {
        m_microDecisionIndex++;
        m_showMicroDecisionComplete = false;
        emit microDecisionChanged();
    }
}

void SessionController::resetMicroDecisions() {
    DecisionEngine::clearAllOptionComparisons(m_session);
    m_microDecisionIndex = 0;
    m_showMicroDecisionComplete = false;
    m_resultsDirty = true;
    emit microDecisionChanged();
    emit resultsChanged();
    autosave();
}

void SessionController::setShowMicroDecisionComplete(bool show) {
    m_showMicroDecisionComplete = show;
    emit microDecisionChanged();
}

void SessionController::recordOptionChoice(int choice) {
    auto questions = DecisionEngine::allOptionMicroDecisions(m_session);
    if (questions.empty()) return;

    int idx = std::max(0, std::min(m_microDecisionIndex, static_cast<int>(questions.size()) - 1));
    const auto& q = questions[idx];

    DecisionEngine::recordOptionComparison(
        m_session,
        q.criterionId,
        q.optionAId,
        q.optionBId,
        choice
    );

    m_resultsDirty = true;
    emit resultsChanged();
    autosave();

    // Advance to next question or show complete
    if (m_microDecisionIndex + 1 < static_cast<int>(questions.size())) {
        m_microDecisionIndex++;
    } else {
        if (microDecisionsComplete()) {
            m_showMicroDecisionComplete = true;
        }
    }

    emit microDecisionChanged();
}

// ── Phase 5: Criteria Weighting ──────────────────────────────────────────────

QString SessionController::criterionPairLeftId() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    return fromId(questions[idx].criterionAId);
}

QString SessionController::criterionPairLeftLabel() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    return QString::fromStdString(questions[idx].criterionALabel);
}

QString SessionController::criterionPairRightId() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    return fromId(questions[idx].criterionBId);
}

QString SessionController::criterionPairRightLabel() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return QString();
    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    return QString::fromStdString(questions[idx].criterionBLabel);
}

QString SessionController::criteriaWeightQuestion() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return QStringLiteral("No weighting questions");
    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    return QString("Pair %1 of %2").arg(idx + 1).arg(questions.size());
}

float SessionController::criteriaWeightProgress() const {
    return DecisionEngine::criteriaWeightingProgress(m_session);
}

bool SessionController::criteriaWeightComplete() const {
    if (m_session.criteria.size() <= 1) return true;
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return true;
    for (const auto& q : questions) {
        if (!q.hasAnswer) return false;
    }
    return true;
}

int SessionController::currentCriteriaAnswer() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return 999;
    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    return questions[idx].currentAnswer;
}

bool SessionController::hasPreviousCriteriaWeight() const {
    return m_criteriaWeightIndex > 0;
}

bool SessionController::hasNextCriteriaWeight() const {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    return m_criteriaWeightIndex + 1 < static_cast<int>(questions.size());
}

void SessionController::previousCriteriaWeight() {
    if (m_criteriaWeightIndex > 0) {
        m_criteriaWeightIndex--;
        m_showCriteriaWeightComplete = false;
        emit criteriaWeightChanged();
    }
}

void SessionController::nextCriteriaWeight() {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (m_criteriaWeightIndex + 1 < static_cast<int>(questions.size())) {
        m_criteriaWeightIndex++;
        m_showCriteriaWeightComplete = false;
        emit criteriaWeightChanged();
    }
}

void SessionController::resetCriteriaWeights() {
    DecisionEngine::clearAllCriteriaComparisons(m_session);
    m_criteriaWeightIndex = 0;
    m_showCriteriaWeightComplete = false;
    m_resultsDirty = true;
    emit criteriaWeightChanged();
    emit resultsChanged();
    autosave();
}

void SessionController::setShowCriteriaWeightComplete(bool show) {
    m_showCriteriaWeightComplete = show;
    emit criteriaWeightChanged();
}

void SessionController::recordCriteriaChoice(int choice) {
    auto questions = DecisionEngine::allCriteriaWeightDecisions(m_session);
    if (questions.empty()) return;

    int idx = std::max(0, std::min(m_criteriaWeightIndex, static_cast<int>(questions.size()) - 1));
    const auto& q = questions[idx];

    DecisionEngine::recordCriteriaComparison(
        m_session,
        q.criterionAId,
        q.criterionBId,
        choice
    );

    m_resultsDirty = true;
    emit resultsChanged();
    autosave();

    // Advance to next question or show complete
    if (m_criteriaWeightIndex + 1 < static_cast<int>(questions.size())) {
        m_criteriaWeightIndex++;
    } else {
        if (criteriaWeightComplete()) {
            m_showCriteriaWeightComplete = true;
        }
    }

    emit criteriaWeightChanged();
}

// ── Phase 6: Results ─────────────────────────────────────────────────────────

QVariantList SessionController::resultsList() const {
    if (m_resultsDirty) {
        m_cachedResults = DecisionEngine::computeRankedResults(m_session);
        m_resultsDirty  = false;
    }

    QVariantList list;
    for (const auto& r : m_cachedResults) {
        QVariantMap m;
        m["rank"]       = r.rank;
        m["optionId"]   = fromId(r.optionId);
        m["label"]      = QString::fromStdString(r.optionLabel);
        m["score"]      = r.totalScore;
        m["percentage"] = static_cast<int>(std::round(r.totalScore * 100.0f));

        QVariantList bList;
        for (const auto& b : r.breakdowns) {
            QVariantMap bm;
            bm["criterionId"]          = fromId(b.criterionId);
            bm["criterionLabel"]       = QString::fromStdString(b.criterionLabel);
            bm["criterionScore"]       = b.criterionScore;
            bm["criterionPercentage"]  = static_cast<int>(std::round(b.criterionScore * 100.0f));
            bm["criterionWeight"]      = b.criterionWeight;
            bm["weightPercentage"]     = static_cast<int>(std::round(b.criterionWeight * 100.0f));
            bm["weightedContribution"] = b.weightedContribution;
            bList.append(bm);
        }
        m["breakdowns"] = bList;
        list.append(m);
    }
    return list;
}

QString SessionController::topOptionLabel() const {
    if (m_resultsDirty) {
        m_cachedResults = DecisionEngine::computeRankedResults(m_session);
        m_resultsDirty  = false;
    }
    return (!m_cachedResults.empty()) ? QString::fromStdString(m_cachedResults.front().optionLabel) : QString();
}

int SessionController::topOptionPercentage() const {
    if (m_resultsDirty) {
        m_cachedResults = DecisionEngine::computeRankedResults(m_session);
        m_resultsDirty  = false;
    }
    return (!m_cachedResults.empty())
        ? static_cast<int>(std::round(m_cachedResults.front().totalScore * 100.0f))
        : 0;
}

QVariantList SessionController::criteriaImportanceList() const {
    auto weights = DecisionEngine::computeCriteriaWeights(m_session);
    QVariantList list;
    for (const auto& c : m_session.criteria) {
        auto it = weights.find(c.id);
        float w = (it != weights.end()) ? it->second : 0.0f;
        QVariantMap m;
        m["id"]         = fromId(c.id);
        m["label"]      = QString::fromStdString(c.label);
        m["weight"]     = w;
        m["percentage"] = static_cast<int>(std::round(w * 100.0f));
        list.append(m);
    }
    return list;
}
