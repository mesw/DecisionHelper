#pragma once
#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVariantMap>
#include <vector>
#include "../core/Types.h"
#include "../core/DecisionEngine.h"

static constexpr const char* kStorageKey = "decisionhelper_session";

class SessionController : public QObject {
    Q_OBJECT

    // ── Phase & Navigation ───────────────────────────────────────────────────
    Q_PROPERTY(int  currentPhase    READ currentPhase    NOTIFY currentPhaseChanged)
    Q_PROPERTY(bool hasSavedSession READ hasSavedSession NOTIFY hasSavedSessionChanged)

    // ── Phase 1: Decision Name ───────────────────────────────────────────────
    Q_PROPERTY(QString decisionTitle       READ decisionTitle       WRITE setDecisionTitle NOTIFY decisionTitleChanged)
    Q_PROPERTY(bool    canProceedToPhase2  READ canProceedToPhase2  NOTIFY decisionTitleChanged)

    // ── Phase 2: Options ─────────────────────────────────────────────────────
    Q_PROPERTY(QVariantList options             READ options             NOTIFY optionsChanged)
    Q_PROPERTY(int          optionCount         READ optionCount         NOTIFY optionsChanged)
    Q_PROPERTY(bool         canProceedToPhase3  READ canProceedToPhase3  NOTIFY optionsChanged)

    // ── Phase 3: Criteria ────────────────────────────────────────────────────
    Q_PROPERTY(QVariantList criteria            READ criteria            NOTIFY criteriaChanged)
    Q_PROPERTY(int          criterionCount      READ criterionCount      NOTIFY criteriaChanged)
    Q_PROPERTY(bool         canProceedToPhase4  READ canProceedToPhase4  NOTIFY criteriaChanged)

    // ── Phase 4: Option Micro-Decisions ──────────────────────────────────────
    Q_PROPERTY(QString currentCriterionLabel       READ currentCriterionLabel       NOTIFY microDecisionChanged)
    Q_PROPERTY(QString optionPairLeftId            READ optionPairLeftId            NOTIFY microDecisionChanged)
    Q_PROPERTY(QString optionPairLeftLabel         READ optionPairLeftLabel         NOTIFY microDecisionChanged)
    Q_PROPERTY(QString optionPairRightId           READ optionPairRightId           NOTIFY microDecisionChanged)
    Q_PROPERTY(QString optionPairRightLabel        READ optionPairRightLabel        NOTIFY microDecisionChanged)
    Q_PROPERTY(QString microDecisionQuestion       READ microDecisionQuestion       NOTIFY microDecisionChanged)
    Q_PROPERTY(float   microDecisionProgress       READ microDecisionProgress       NOTIFY microDecisionChanged)
    Q_PROPERTY(bool    microDecisionsComplete      READ microDecisionsComplete      NOTIFY microDecisionChanged)
    Q_PROPERTY(int     currentOptionAnswer         READ currentOptionAnswer         NOTIFY microDecisionChanged)
    Q_PROPERTY(bool    hasPreviousMicroDecision    READ hasPreviousMicroDecision    NOTIFY microDecisionChanged)
    Q_PROPERTY(bool    hasNextMicroDecision        READ hasNextMicroDecision        NOTIFY microDecisionChanged)
    Q_PROPERTY(bool    showMicroDecisionComplete   READ showMicroDecisionComplete   NOTIFY microDecisionChanged)

    // ── Phase 5: Criteria Weighting ──────────────────────────────────────────
    Q_PROPERTY(QString criterionPairLeftId         READ criterionPairLeftId         NOTIFY criteriaWeightChanged)
    Q_PROPERTY(QString criterionPairLeftLabel      READ criterionPairLeftLabel      NOTIFY criteriaWeightChanged)
    Q_PROPERTY(QString criterionPairRightId        READ criterionPairRightId        NOTIFY criteriaWeightChanged)
    Q_PROPERTY(QString criterionPairRightLabel     READ criterionPairRightLabel     NOTIFY criteriaWeightChanged)
    Q_PROPERTY(QString criteriaWeightQuestion      READ criteriaWeightQuestion      NOTIFY criteriaWeightChanged)
    Q_PROPERTY(float   criteriaWeightProgress      READ criteriaWeightProgress      NOTIFY criteriaWeightChanged)
    Q_PROPERTY(bool    criteriaWeightComplete      READ criteriaWeightComplete      NOTIFY criteriaWeightChanged)
    Q_PROPERTY(int     currentCriteriaAnswer       READ currentCriteriaAnswer       NOTIFY criteriaWeightChanged)
    Q_PROPERTY(bool    hasPreviousCriteriaWeight   READ hasPreviousCriteriaWeight   NOTIFY criteriaWeightChanged)
    Q_PROPERTY(bool    hasNextCriteriaWeight       READ hasNextCriteriaWeight       NOTIFY criteriaWeightChanged)
    Q_PROPERTY(bool    showCriteriaWeightComplete  READ showCriteriaWeightComplete  NOTIFY criteriaWeightChanged)

    // ── Phase 6: Results ─────────────────────────────────────────────────────
    Q_PROPERTY(QVariantList resultsList            READ resultsList            NOTIFY resultsChanged)
    Q_PROPERTY(QString      topOptionLabel         READ topOptionLabel         NOTIFY resultsChanged)
    Q_PROPERTY(int          topOptionPercentage    READ topOptionPercentage    NOTIFY resultsChanged)
    Q_PROPERTY(QVariantList criteriaImportanceList READ criteriaImportanceList NOTIFY resultsChanged)

public:
    explicit SessionController(QObject* parent = nullptr);

    int  currentPhase()    const { return m_phase; }
    bool hasSavedSession() const;

    // Phase 1
    QString decisionTitle()      const;
    bool    canProceedToPhase2() const;

    // Phase 2
    QVariantList options()            const;
    int          optionCount()        const { return static_cast<int>(m_session.options.size()); }
    bool         canProceedToPhase3() const { return optionCount() >= 2; }

    // Phase 3
    QVariantList criteria()           const;
    int          criterionCount()     const { return static_cast<int>(m_session.criteria.size()); }
    bool         canProceedToPhase4() const { return criterionCount() >= 1; }

    // Phase 4
    QString currentCriterionLabel()      const;
    QString optionPairLeftId()           const;
    QString optionPairLeftLabel()        const;
    QString optionPairRightId()          const;
    QString optionPairRightLabel()       const;
    QString microDecisionQuestion()      const;
    float   microDecisionProgress()      const;
    bool    microDecisionsComplete()     const;
    int     currentOptionAnswer()        const;
    bool    hasPreviousMicroDecision()   const;
    bool    hasNextMicroDecision()       const;
    bool    showMicroDecisionComplete()  const { return m_showMicroDecisionComplete; }

    // Phase 5
    QString criterionPairLeftId()        const;
    QString criterionPairLeftLabel()     const;
    QString criterionPairRightId()       const;
    QString criterionPairRightLabel()    const;
    QString criteriaWeightQuestion()     const;
    float   criteriaWeightProgress()     const;
    bool    criteriaWeightComplete()     const;
    int     currentCriteriaAnswer()      const;
    bool    hasPreviousCriteriaWeight()  const;
    bool    hasNextCriteriaWeight()      const;
    bool    showCriteriaWeightComplete() const { return m_showCriteriaWeightComplete; }

    // Phase 6
    QVariantList resultsList()            const;
    QString      topOptionLabel()         const;
    int          topOptionPercentage()    const;
    QVariantList criteriaImportanceList() const;

signals:
    void currentPhaseChanged();
    void hasSavedSessionChanged();
    void decisionTitleChanged();
    void optionsChanged();
    void criteriaChanged();
    void microDecisionChanged();
    void criteriaWeightChanged();
    void resultsChanged();

public slots:
    // Navigation
    Q_INVOKABLE void goToPhase(int phase);
    Q_INVOKABLE void nextPhase();
    Q_INVOKABLE void previousPhase();

    // Phase 1 & Setup
    Q_INVOKABLE void setDecisionTitle(const QString& title);
    Q_INVOKABLE void newSession();
    Q_INVOKABLE void loadSession();
    Q_INVOKABLE void saveSession();

    // Phase 2: Options
    Q_INVOKABLE void addOption(const QString& label);
    Q_INVOKABLE void removeOption(const QString& id);

    // Phase 3: Criteria
    Q_INVOKABLE void addCriterion(const QString& label);
    Q_INVOKABLE void removeCriterion(const QString& id);

    // Phase 4: Option Micro-Decisions
    // choice: +1 = Left card wins, -1 = Right card wins, 0 = Equal / Tie
    Q_INVOKABLE void recordOptionChoice(int choice);
    Q_INVOKABLE void previousMicroDecision();
    Q_INVOKABLE void nextMicroDecision();
    Q_INVOKABLE void resetMicroDecisions();
    Q_INVOKABLE void setShowMicroDecisionComplete(bool show);

    // Phase 5: Criteria Weighting
    // choice: +1 = Left criterion wins, -1 = Right criterion wins, 0 = Equal / Tie
    Q_INVOKABLE void recordCriteriaChoice(int choice);
    Q_INVOKABLE void previousCriteriaWeight();
    Q_INVOKABLE void nextCriteriaWeight();
    Q_INVOKABLE void resetCriteriaWeights();
    Q_INVOKABLE void setShowCriteriaWeightComplete(bool show);

    // Review / Adjustment triggers from Phase 6
    Q_INVOKABLE void adjustMicroDecisions();
    Q_INVOKABLE void adjustCriteriaWeights();

private:
    void autosave();
    void refreshPhaseState();

    DecisionSession m_session;
    int             m_phase = 1;

    // Phase 4 state
    int  m_microDecisionIndex = 0;
    bool m_showMicroDecisionComplete = false;

    // Phase 5 state
    int  m_criteriaWeightIndex = 0;
    bool m_showCriteriaWeightComplete = false;

    // Phase 6 cached results
    mutable std::vector<OptionResult> m_cachedResults;
    mutable bool                      m_resultsDirty = true;
};
