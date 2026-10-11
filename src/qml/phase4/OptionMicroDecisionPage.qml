import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: root

    header: AppHeader {
        title: "Step 4: Micro-Decisions"
        subtitle: sessionController.decisionTitle
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Active criterion banner
        Rectangle {
            Layout.fillWidth: true
            height: 64
            radius: 10
            color: Material.color(Material.Blue, Material.Shade50)
            border.color: Material.color(Material.Blue, Material.Shade200)

            ColumnLayout {
                anchors.centerIn: parent
                spacing: 2

                Label {
                    text: "EVALUATING CRITERION"
                    font.pixelSize: 11
                    font.bold: true
                    color: Material.accent
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: sessionController.currentCriterionLabel
                    font.pixelSize: 18
                    font.bold: true
                    color: Material.foreground
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }

        // Progress & Question indicator
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: sessionController.microDecisionQuestion
                font.pixelSize: 13
                color: Material.color(Material.Grey)
            }

            Item { Layout.fillWidth: true }

            Label {
                text: Math.round(sessionController.microDecisionProgress * 100) + "%"
                font.pixelSize: 13
                font.bold: true
                color: Material.accent
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            value: sessionController.microDecisionProgress
        }

        // Comparison view / Completion view
        Loader {
            id: contentLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: sessionController.showMicroDecisionComplete ? completeComponent : decisionComponent
        }

        // Component 1: Active 3-Way Micro-Decision Cards
        Component {
            id: decisionComponent

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                Label {
                    text: "Which option is better regarding " + sessionController.currentCriterionLabel + "?"
                    font.pixelSize: 15
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                // 3 equally sized and accentuated cards with isSelected feedback
                DecisionChoiceCard {
                    title: sessionController.optionPairLeftLabel
                    subtitle: "Select if preferred"
                    isHighlighted: true
                    isSelected: sessionController.currentOptionAnswer === 1
                    onClicked: sessionController.recordOptionChoice(1)
                }

                DecisionChoiceCard {
                    title: "Equally Good"
                    subtitle: "Both are about the same"
                    isHighlighted: true
                    accentColor: Material.color(Material.Teal)
                    isSelected: sessionController.currentOptionAnswer === 0
                    onClicked: sessionController.recordOptionChoice(0)
                }

                DecisionChoiceCard {
                    title: sessionController.optionPairRightLabel
                    subtitle: "Select if preferred"
                    isHighlighted: true
                    isSelected: sessionController.currentOptionAnswer === -1
                    onClicked: sessionController.recordOptionChoice(-1)
                }

                // In-card navigation bar (Previous / Next / Done)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        text: "← Prev"
                        flat: true
                        enabled: sessionController.hasPreviousMicroDecision
                        onClicked: sessionController.previousMicroDecision()
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Done / Results →"
                        visible: sessionController.microDecisionsComplete
                        Material.background: Material.Blue
                        Material.foreground: "white"
                        onClicked: sessionController.goToPhase(sessionController.criteriaWeightComplete ? 6 : 5)
                    }

                    Button {
                        text: "Next →"
                        flat: true
                        enabled: sessionController.hasNextMicroDecision
                        onClicked: sessionController.nextMicroDecision()
                    }
                }
            }
        }

        // Component 2: Micro-Decisions Completed
        Component {
            id: completeComponent

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                Item { Layout.fillHeight: true }

                Label {
                    text: "✓"
                    font.pixelSize: 48
                    color: Material.color(Material.Green)
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Micro-Decisions Completed!"
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "All options have been evaluated across each criterion."
                    font.pixelSize: 14
                    color: Material.color(Material.Grey)
                    Layout.alignment: Qt.AlignHCenter
                }

                Item { height: 12 }

                Button {
                    text: sessionController.criteriaWeightComplete
                        ? "View Decision Results →"
                        : "Continue to Criteria Weighting →"
                    font.pixelSize: 16
                    Layout.preferredHeight: 48
                    Layout.fillWidth: true
                    Layout.maximumWidth: 360
                    Layout.alignment: Qt.AlignHCenter
                    Material.background: Material.Blue
                    Material.foreground: "white"
                    onClicked: sessionController.goToPhase(sessionController.criteriaWeightComplete ? 6 : 5)
                }

                Button {
                    text: "Review / Adjust Answers"
                    flat: true
                    Layout.fillWidth: true
                    Layout.maximumWidth: 360
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: sessionController.setShowMicroDecisionComplete(false)
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Bottom navigation
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "← Back to Criteria"
                flat: true
                onClicked: sessionController.goToPhase(3)
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Reset All Answers"
                flat: true
                visible: sessionController.microDecisionProgress > 0
                onClicked: sessionController.resetMicroDecisions()
            }
        }
    }
}
