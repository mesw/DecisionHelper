import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: root

    header: AppHeader {
        title: "Step 3: Define Criteria"
        subtitle: sessionController.decisionTitle
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: "What criteria or factors matter for this decision?"
            font.pixelSize: 16
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: criterionInput
                placeholderText: "e.g. Price, Quality, Comfort, Delivery Time"
                font.pixelSize: 15
                Layout.fillWidth: true
                onAccepted: addCurrentCriterion()
            }

            Button {
                text: "Add"
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: addCurrentCriterion()
            }
        }

        ListView {
            id: criteriaListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sessionController.criteria
            spacing: 6
            clip: true

            delegate: Rectangle {
                id: delegateRect
                width: criteriaListView.width
                height: 48
                radius: 8
                color: Material.color(Material.Grey, Material.Shade100)
                border.color: Material.color(Material.Grey, Material.Shade300)

                required property var modelData

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 12
                    anchors.rightMargin: 8

                    Label {
                        text: "◆"
                        color: Material.accent
                        font.pixelSize: 14
                    }

                    Label {
                        text: delegateRect.modelData.label
                        font.pixelSize: 16
                        Layout.fillWidth: true
                        elide: Text.ElideRight
                    }

                    Button {
                        flat: true
                        text: "✕"
                        font.pixelSize: 16
                        implicitWidth: 36
                        implicitHeight: 36
                        onClicked: sessionController.removeCriterion(delegateRect.modelData.id)
                    }
                }
            }
        }

        Label {
            visible: !sessionController.canProceedToPhase4
            text: "Add at least 1 criterion to proceed."
            color: Material.color(Material.Orange, Material.Shade800)
            font.pixelSize: 13
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 12

            Button {
                text: "← Back"
                flat: true
                Layout.preferredWidth: 100
                onClicked: sessionController.goToPhase(2)
            }

            Button {
                text: "Start Micro-Decisions →"
                Layout.fillWidth: true
                enabled: sessionController.canProceedToPhase4
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: sessionController.goToPhase(4)
            }
        }
    }

    function addCurrentCriterion() {
        var t = criterionInput.text.trim()
        if (t.length > 0) {
            sessionController.addCriterion(t)
            criterionInput.text = ""
        }
    }
}
