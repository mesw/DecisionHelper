import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: root

    header: AppHeader {
        title: "Step 2: Enter Options"
        subtitle: sessionController.decisionTitle
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 16
        spacing: 12

        Label {
            text: "What options are you evaluating?"
            font.pixelSize: 16
            font.bold: true
        }

        RowLayout {
            Layout.fillWidth: true
            spacing: 8

            TextField {
                id: optionInput
                placeholderText: "e.g. Option A, Pizza, Lenovo X1"
                font.pixelSize: 15
                Layout.fillWidth: true
                onAccepted: addCurrentOption()
            }

            Button {
                text: "Add"
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: addCurrentOption()
            }
        }

        ListView {
            id: optionsListView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sessionController.options
            spacing: 6
            clip: true

            delegate: Rectangle {
                id: delegateRect
                width: optionsListView.width
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
                        text: "●"
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
                        onClicked: sessionController.removeOption(delegateRect.modelData.id)
                    }
                }
            }
        }

        Label {
            visible: !sessionController.canProceedToPhase3
            text: "Add at least 2 options to proceed (" + sessionController.optionCount + "/2 added)."
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
                onClicked: sessionController.goToPhase(1)
            }

            Button {
                text: "Next: Define Criteria →"
                Layout.fillWidth: true
                enabled: sessionController.canProceedToPhase3
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: sessionController.goToPhase(3)
            }
        }
    }

    function addCurrentOption() {
        var t = optionInput.text.trim()
        if (t.length > 0) {
            sessionController.addOption(t)
            optionInput.text = ""
        }
    }
}
