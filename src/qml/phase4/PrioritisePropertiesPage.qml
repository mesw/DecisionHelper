import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    header: AppHeader {
        title: "Prioritise Criteria"
        subtitle: "Drag to rank — most important first"
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        Label {
            text: "Drag to reorder. Top = most important."
            font.pixelSize: 13
            color: Material.color(Material.Grey)
        }

        ListView {
            id: dragList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sessionController.properties
            spacing: 4
            clip: true

            delegate: Item {
                id: delegate
                width: dragList.width
                height: 56

                required property int index
                required property var modelData
                property int  myIndex: index

                Rectangle {
                    anchors.fill: parent
                    radius: 8
                    color: dragArea.pressed ? Material.color(Material.Blue, Material.Shade100)
                                            : Material.background
                    border.color: Material.accent
                    border.width: 1

                    Row {
                        anchors { verticalCenter: parent.verticalCenter; left: parent.left; leftMargin: 12 }
                        spacing: 12
                        Label {
                            text: (modelData.declaredPriority > 0 ? modelData.declaredPriority : "?") + "."
                            font.pixelSize: 15
                            font.bold: true
                            color: Material.accent
                            width: 30
                        }
                        Label {
                            text: modelData.label
                            font.pixelSize: 16
                        }
                    }

                    Label {
                        text: "⠿"
                        font.pixelSize: 22
                        color: Material.color(Material.Grey)
                        anchors { right: parent.right; rightMargin: 12; verticalCenter: parent.verticalCenter }
                    }
                }

                MouseArea {
                    id: dragArea
                    anchors.fill: parent
                    drag.target: parent
                    drag.axis: Drag.YAxis

                    onReleased: {
                        var newIndex = Math.floor((delegate.y + delegate.height / 2) / (delegate.height + dragList.spacing))
                        newIndex = Math.max(0, Math.min(dragList.count - 1, newIndex))
                        if (newIndex !== delegate.myIndex)
                            sessionController.reorderProperty(delegate.myIndex, newIndex)
                        delegate.y = delegate.myIndex * (delegate.height + dragList.spacing)
                    }
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Continue to Attribution  →"
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: sessionController.finishPrioritise()
        }
    }
}
