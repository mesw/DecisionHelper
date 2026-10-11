import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: root

    header: AppHeader {
        title: "Step 5: Criteria Weighting"
        subtitle: sessionController.decisionTitle
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        // Progress indicators
        RowLayout {
            Layout.fillWidth: true

            Label {
                text: sessionController.criterionCount <= 1
                    ? "Single criterion mode"
                    : sessionController.criteriaWeightQuestion
                font.pixelSize: 13
                color: Material.color(Material.Grey)
            }

            Item { Layout.fillWidth: true }

            Label {
                text: Math.round(sessionController.criteriaWeightProgress * 100) + "%"
                font.pixelSize: 13
                font.bold: true
                color: Material.accent
            }
        }

        ProgressBar {
            Layout.fillWidth: true
            value: sessionController.criteriaWeightProgress
        }

        // Comparison view / Completion view
        Loader {
            id: contentLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: {
                if (sessionController.criterionCount <= 1) return singleCriterionComponent
                if (sessionController.showCriteriaWeightComplete) return completeComponent
                return decisionComponent
            }
        }

        // Component 1: Active 3-Way Criteria Comparison
        Component {
            id: decisionComponent

            ColumnLayout {
                anchors.fill: parent
                spacing: 12

                Label {
                    text: "Which criterion matters more to you for this decision?"
                    font.pixelSize: 15
                    horizontalAlignment: Text.AlignHCenter
                    Layout.fillWidth: true
                    wrapMode: Text.WordWrap
                }

                // 3 equally sized and accentuated cards
                DecisionChoiceCard {
                    title: sessionController.criterionPairLeftLabel
                    subtitle: "Matters more"
                    isHighlighted: true
                    isSelected: sessionController.currentCriteriaAnswer === 1
                    onClicked: sessionController.recordCriteriaChoice(1)
                }

                DecisionChoiceCard {
                    title: "Equally Important"
                    subtitle: "Both matter the same"
                    isHighlighted: true
                    accentColor: Material.color(Material.Teal)
                    isSelected: sessionController.currentCriteriaAnswer === 0
                    onClicked: sessionController.recordCriteriaChoice(0)
                }

                DecisionChoiceCard {
                    title: sessionController.criterionPairRightLabel
                    subtitle: "Matters more"
                    isHighlighted: true
                    isSelected: sessionController.currentCriteriaAnswer === -1
                    onClicked: sessionController.recordCriteriaChoice(-1)
                }

                // In-card navigation bar (Previous / Next / Done)
                RowLayout {
                    Layout.fillWidth: true
                    spacing: 8

                    Button {
                        text: "← Prev"
                        flat: true
                        enabled: sessionController.hasPreviousCriteriaWeight
                        onClicked: sessionController.previousCriteriaWeight()
                    }

                    Item { Layout.fillWidth: true }

                    Button {
                        text: "Done / Results →"
                        visible: sessionController.criteriaWeightComplete
                        Material.background: Material.Blue
                        Material.foreground: "white"
                        onClicked: sessionController.goToPhase(6)
                    }

                    Button {
                        text: "Next →"
                        flat: true
                        enabled: sessionController.hasNextCriteriaWeight
                        onClicked: sessionController.nextCriteriaWeight()
                    }
                }
            }
        }

        // Component 2: Single Criterion
        Component {
            id: singleCriterionComponent

            ColumnLayout {
                anchors.fill: parent
                spacing: 16

                Item { Layout.fillHeight: true }

                Label {
                    text: "ℹ"
                    font.pixelSize: 48
                    color: Material.accent
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Only 1 Criterion Defined"
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Since only one criterion was defined, it automatically receives 100% of the decision weight."
                    font.pixelSize: 14
                    color: Material.color(Material.Grey)
                    wrapMode: Text.WordWrap
                    Layout.alignment: Qt.AlignHCenter
                }

                Item { height: 16 }

                Button {
                    text: "View Decision Results →"
                    font.pixelSize: 16
                    Layout.preferredHeight: 48
                    Layout.fillWidth: true
                    Layout.maximumWidth: 360
                    Layout.alignment: Qt.AlignHCenter
                    Material.background: Material.Blue
                    Material.foreground: "white"
                    onClicked: sessionController.goToPhase(6)
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Component 3: Weighting Completed
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
                    text: "Criteria Weighting Completed!"
                    font.pixelSize: 20
                    font.bold: true
                    Layout.alignment: Qt.AlignHCenter
                }

                Label {
                    text: "Your relative priorities have been captured."
                    font.pixelSize: 14
                    color: Material.color(Material.Grey)
                    Layout.alignment: Qt.AlignHCenter
                }

                Item { height: 12 }

                Button {
                    text: "View Decision Results →"
                    font.pixelSize: 16
                    Layout.preferredHeight: 48
                    Layout.fillWidth: true
                    Layout.maximumWidth: 360
                    Layout.alignment: Qt.AlignHCenter
                    Material.background: Material.Blue
                    Material.foreground: "white"
                    onClicked: sessionController.goToPhase(6)
                }

                Button {
                    text: "Review / Adjust Weights"
                    flat: true
                    Layout.fillWidth: true
                    Layout.maximumWidth: 360
                    Layout.alignment: Qt.AlignHCenter
                    onClicked: sessionController.setShowCriteriaWeightComplete(false)
                }

                Item { Layout.fillHeight: true }
            }
        }

        // Bottom navigation
        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "← Back to Micro-Decisions"
                flat: true
                onClicked: sessionController.goToPhase(4)
            }

            Item { Layout.fillWidth: true }

            Button {
                text: "Reset All Weights"
                flat: true
                visible: sessionController.criteriaWeightProgress > 0 && sessionController.criterionCount > 1
                onClicked: sessionController.resetCriteriaWeights()
            }
        }
    }
}
