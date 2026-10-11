import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    header: AppHeader {
        title: "Add Options"
        subtitle: "What are you choosing between?"
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            TextField {
                id: input
                placeholderText: "e.g. Pizza"
                Layout.fillWidth: true
                onAccepted: addOption()
            }
            Button {
                id: addBtn
                text: "Add"
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: addOption()
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sessionController.options
            spacing: 4
            clip: true

            delegate: ItemDelegate {
                id: del
                width: listView.width
                required property var modelData

                contentItem: Label {
                    text: del.modelData.label
                    font.pixelSize: 16
                    anchors.verticalCenter: parent.verticalCenter
                }

                Button {
                    flat: true
                    text: "×"
                    font.pixelSize: 18
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                    onClicked: sessionController.removeOption(del.modelData.id)
                }
            }
        }

        Label {
            visible: sessionController.optionCount < 2
            text: "Add at least 2 options to continue."
            color: Material.color(Material.Grey)
            font.pixelSize: 12
        }

        Button {
            Layout.fillWidth: true
            text: "Start Ranking  →"
            enabled: sessionController.optionCount >= 2
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: sessionController.beginRanking()
        }
    }

    function addOption() {
        var t = input.text.trim()
        if (t.length > 0) {
            sessionController.addOption(t)
            input.text = ""
        }
    }
}
