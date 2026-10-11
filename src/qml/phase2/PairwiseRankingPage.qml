import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: page
    header: AppHeader {
        title: insertionMode ? "Insert Option" : "Rank Options"
        subtitle: insertionMode
            ? "Step " + sessionController.insertionStepCurrent + " of ≤" + sessionController.insertionStepMax
            : "Gut feel only — which do you prefer?"
    }

    property bool insertionMode: sessionController.insertionActive

    // Show completion page if ranking is done and not in insertion mode
    Loader {
        id: contentLoader
        anchors.fill: parent
        sourceComponent: {
            if (!insertionMode && sessionController.rankingComplete) return resultComp
            return pairsComp
        }
    }

    // ── Pairwise comparison ──────────────────────────────────────────────────
    Component {
        id: pairsComp
        Item {
            property bool portrait: height > width

            // Left/Top card
            OptionCard {
                id: leftCard
                optionLabel: sessionController.rankingPairLeftLabel
                anchors {
                    left: parent.left; right: portrait ? parent.right : parent.horizontalCenter
                    top: parent.top;   bottom: portrait ? parent.verticalCenter : parent.bottom
                    margins: 16; rightMargin: portrait ? 16 : 8; bottomMargin: portrait ? 8 : 16
                }
                onChosen: choose(sessionController.rankingPairLeftId,
                                 sessionController.rankingPairRightId)
            }

            // Right/Bottom card
            OptionCard {
                id: rightCard
                optionLabel: sessionController.rankingPairRightLabel
                isLeft: false
                anchors {
                    left: portrait ? parent.left : parent.horizontalCenter
                    right: parent.right
                    top: portrait ? parent.verticalCenter : parent.top
                    bottom: parent.bottom
                    margins: 16; leftMargin: portrait ? 16 : 8; topMargin: portrait ? 8 : 16
                }
                onChosen: choose(sessionController.rankingPairRightId,
                                 sessionController.rankingPairLeftId)
            }

            // Tie button
            Button {
                flat: true
                text: "It's a tie"
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                z: 10
                onClicked: {
                    if (page.insertionMode)
                        sessionController.recordInsertionComparison(
                            sessionController.rankingPairRightId, 0)
                    else
                        sessionController.recordTie(
                            sessionController.rankingPairLeftId,
                            sessionController.rankingPairRightId)
                }
            }

            // Progress bar at bottom
            SweetSpotProgressBar {
                anchors { bottom: parent.bottom; left: parent.left; right: parent.right; margins: 16 }
                value: sessionController.rankingProgress
            }

            function choose(winnerId, loserId) {
                if (page.insertionMode)
                    sessionController.recordInsertionComparison(loserId, 1)
                else
                    sessionController.recordComparison(winnerId, loserId)
            }
        }
    }

    // ── Result after ranking complete ────────────────────────────────────────
    Component {
        id: resultComp
        RankingResultPage {}
    }
}
