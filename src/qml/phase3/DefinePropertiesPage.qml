import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: page
    property bool loopMode: false

    header: AppHeader {
        title: loopMode ? "Add More Criteria" : "Define Criteria"
        subtitle: "What matters in this decision?"
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        RowLayout {
            Layout.fillWidth: true
            spacing: 8
            TextField {
                id: input
                placeholderText: "e.g. Price, Healthiness"
                Layout.fillWidth: true
                onAccepted: addProp()
            }
            Button {
                id: addBtn
                text: "Add"
                Material.background: Material.Blue
                Material.foreground: "white"
                onClicked: addProp()
            }
        }

        ListView {
            id: listView
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sessionController.properties
            spacing: 4
            clip: true
            delegate: ItemDelegate {
                id: del
                width: listView.width
                required property var modelData
                property bool isExisting: page.loopMode && del.modelData.declaredPriority > 0

                contentItem: Row {
                    spacing: 8
                    Label {
                        text: del.modelData.label
                        font.pixelSize: 16
                        opacity: del.isExisting ? 0.4 : 1.0
                        anchors.verticalCenter: parent.verticalCenter
                    }
                    Label {
                        visible: del.isExisting
                        text: "(existing)"
                        font.pixelSize: 12
                        color: Material.color(Material.Grey)
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Button {
                    flat: true; text: "×"; font.pixelSize: 18
                    anchors { right: parent.right; verticalCenter: parent.verticalCenter }
                    visible: !del.isExisting
                    onClicked: sessionController.removeProperty(del.modelData.id)
                }
            }
        }

        Label {
            visible: sessionController.propertyCount < 1
            text: "Add at least 1 criterion to continue."
            color: Material.color(Material.Grey)
            font.pixelSize: 12
        }

        Button {
            Layout.fillWidth: true
            text: loopMode ? "Go to Attribution  →" : "Prioritise Criteria  →"
            enabled: sessionController.propertyCount >= 1
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: {
                if (page.loopMode) sessionController.goToPhase(5)
                else               sessionController.beginPrioritise()
            }
        }
    }

    function addProp() {
        var t = input.text.trim()
        if (t.length > 0) {
            sessionController.addProperty(t)
            input.text = ""
        }
    }
}
