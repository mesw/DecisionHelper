import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: root

    header: AppHeader {
        id: appHeader
        title: "Step 6: Decision Results"
        subtitle: sessionController.decisionTitle
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth
        clip: true

        ColumnLayout {
            width: root.width - 32
            anchors.horizontalCenter: parent.horizontalCenter
            spacing: 20

            Item { height: 4 } // Top padding

            // 🏆 Recommended Winner Card
            Rectangle {
                Layout.fillWidth: true
                radius: 12
                color: Material.color(Material.Amber, Material.Shade50)
                border.color: Material.color(Material.Amber, Material.Shade400)
                border.width: 2
                implicitHeight: winnerCol.implicitHeight + 24

                ColumnLayout {
                    id: winnerCol
                    anchors { fill: parent; margins: 16 }
                    spacing: 6

                    RowLayout {
                        spacing: 6
                        Label {
                            text: "🏆"
                            font.pixelSize: 16
                            font.family: typeof emojiFontFamily !== "undefined" && emojiFontFamily.length > 0 ? emojiFontFamily : "Noto Color Emoji"
                        }
                        Label {
                            text: "RECOMMENDED CHOICE"
                            font.pixelSize: 12
                            font.bold: true
                            color: Material.color(Material.Amber, Material.Shade900)
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        Label {
                            text: sessionController.topOptionLabel
                            font.pixelSize: 22
                            font.bold: true
                            Layout.fillWidth: true
                            elide: Text.ElideRight
                        }
                        Label {
                            text: sessionController.topOptionPercentage + "%"
                            font.pixelSize: 24
                            font.bold: true
                            color: Material.accent
                        }
                    }
                }
            }

            // Section: All Ranked Options
            Label {
                text: "Ranked Options"
                font.pixelSize: 16
                font.bold: true
            }

            Repeater {
                model: sessionController.resultsList

                delegate: Rectangle {
                    id: optCard
                    Layout.fillWidth: true
                    radius: 10
                    color: Material.color(Material.Grey, Material.Shade50)
                    border.color: Material.color(Material.Grey, Material.Shade300)
                    implicitHeight: cardCol.implicitHeight + 20

                    required property var modelData

                    ColumnLayout {
                        id: cardCol
                        anchors { fill: parent; margins: 12 }
                        spacing: 8

                        RowLayout {
                            Layout.fillWidth: true

                            Label {
                                text: "#" + optCard.modelData.rank
                                font.pixelSize: 16
                                font.bold: true
                                color: Material.accent
                                Layout.preferredWidth: 28
                            }

                            Label {
                                text: optCard.modelData.label
                                font.pixelSize: 16
                                font.bold: true
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: optCard.modelData.percentage + "%"
                                font.pixelSize: 16
                                font.bold: true
                            }
                        }

                        // Score progress bar
                        ProgressBar {
                            Layout.fillWidth: true
                            value: optCard.modelData.score
                        }

                        // Breakdown per criterion
                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 4

                            Repeater {
                                model: optCard.modelData.breakdowns

                                delegate: RowLayout {
                                    id: breakdownRow
                                    Layout.fillWidth: true
                                    required property var modelData

                                    Label {
                                        text: "• " + breakdownRow.modelData.criterionLabel + ":"
                                        font.pixelSize: 12
                                        color: Material.color(Material.Grey)
                                        Layout.preferredWidth: 120
                                        elide: Text.ElideRight
                                    }

                                    Label {
                                        text: breakdownRow.modelData.criterionPercentage + "% on criterion"
                                        font.pixelSize: 12
                                        color: Material.color(Material.Grey, Material.Shade800)
                                        Layout.fillWidth: true
                                    }

                                    Label {
                                        text: "Weight: " + breakdownRow.modelData.weightPercentage + "%"
                                        font.pixelSize: 11
                                        color: Material.color(Material.Grey)
                                    }
                                }
                            }
                        }
                    }
                }
            }

            // Section: Criteria Importance Breakdown
            Label {
                text: "Criteria Importance"
                font.pixelSize: 16
                font.bold: true
            }

            Rectangle {
                Layout.fillWidth: true
                radius: 10
                color: Material.color(Material.Grey, Material.Shade50)
                border.color: Material.color(Material.Grey, Material.Shade300)
                implicitHeight: critListCol.implicitHeight + 20

                ColumnLayout {
                    id: critListCol
                    anchors { fill: parent; margins: 12 }
                    spacing: 8

                    Repeater {
                        model: sessionController.criteriaImportanceList

                        delegate: RowLayout {
                            id: critRow
                            Layout.fillWidth: true
                            required property var modelData

                            Label {
                                text: "◆ " + critRow.modelData.label
                                font.pixelSize: 14
                                Layout.fillWidth: true
                                elide: Text.ElideRight
                            }

                            Label {
                                text: critRow.modelData.percentage + "% weight"
                                font.pixelSize: 14
                                font.bold: true
                                color: Material.accent
                            }
                        }
                    }
                }
            }

            // Section: Refine Decision
            Label {
                text: "Refine Decision"
                font.pixelSize: 16
                font.bold: true
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "+ Add Option"
                    Layout.fillWidth: true
                    Material.background: Material.color(Material.Blue, Material.Shade50)
                    Material.foreground: Material.accent
                    onClicked: addOptionDialog.open()
                }

                Button {
                    text: "+ Add Criterion"
                    Layout.fillWidth: true
                    Material.background: Material.color(Material.Blue, Material.Shade50)
                    Material.foreground: Material.accent
                    onClicked: addCriterionDialog.open()
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Manage Options"
                    flat: true
                    Layout.fillWidth: true
                    onClicked: sessionController.goToPhase(2)
                }

                Button {
                    text: "Manage Criteria"
                    flat: true
                    Layout.fillWidth: true
                    onClicked: sessionController.goToPhase(3)
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Adjust Micro-Decisions"
                    flat: true
                    Layout.fillWidth: true
                    onClicked: sessionController.adjustMicroDecisions()
                }

                Button {
                    text: "Adjust Criteria Weights"
                    flat: true
                    Layout.fillWidth: true
                    onClicked: sessionController.adjustCriteriaWeights()
                }
            }

            // Bottom Actions
            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Button {
                    text: "Start New Decision"
                    flat: true
                    Layout.fillWidth: true
                    onClicked: sessionController.newSession()
                }

                Button {
                    text: "Adjust Answers"
                    Material.background: Material.Blue
                    Material.foreground: "white"
                    Layout.fillWidth: true
                    onClicked: sessionController.adjustMicroDecisions()
                }
            }

            Button {
                text: "Free & Open Source Software • Licensed under GNU GPL v3.0 (License Info)"
                flat: true
                font.pixelSize: 11
                Material.foreground: Material.color(Material.Grey, Material.Shade600)
                Layout.alignment: Qt.AlignHCenter
                onClicked: appHeader.openLicenseDialog()
            }

            Item { height: 16 } // Bottom spacer
        }
    }

    // Dialog: Add Option
    Dialog {
        id: addOptionDialog
        title: "Add Option"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: Math.min(root.width - 32, 360)

        ColumnLayout {
            width: parent.width
            spacing: 8

            Label {
                text: "Enter a new option to evaluate:"
                font.pixelSize: 13
                color: Material.color(Material.Grey)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: newOptionInput
                placeholderText: "e.g. Option D"
                Layout.fillWidth: true
                focus: true
                onAccepted: {
                    if (newOptionInput.text.trim().length > 0) {
                        addOptionDialog.accept()
                    }
                }
            }
        }

        onAccepted: {
            var val = newOptionInput.text.trim()
            if (val.length > 0) {
                sessionController.addOption(val)
                newOptionInput.text = ""
                sessionController.goToPhase(4)
            }
        }
        onRejected: {
            newOptionInput.text = ""
        }
    }

    // Dialog: Add Criterion
    Dialog {
        id: addCriterionDialog
        title: "Add Criterion"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Ok | Dialog.Cancel
        width: Math.min(root.width - 32, 360)

        ColumnLayout {
            width: parent.width
            spacing: 8

            Label {
                text: "Enter a new comparison criterion:"
                font.pixelSize: 13
                color: Material.color(Material.Grey)
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            TextField {
                id: newCriterionInput
                placeholderText: "e.g. Safety, Warranty, Speed"
                Layout.fillWidth: true
                focus: true
                onAccepted: {
                    if (newCriterionInput.text.trim().length > 0) {
                        addCriterionDialog.accept()
                    }
                }
            }
        }

        onAccepted: {
            var val = newCriterionInput.text.trim()
            if (val.length > 0) {
                sessionController.addCriterion(val)
                newCriterionInput.text = ""
                sessionController.goToPhase(4)
            }
        }
        onRejected: {
            newCriterionInput.text = ""
        }
    }
}
