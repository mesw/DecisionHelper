import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {
    ColumnLayout {
        anchors { fill: parent; margins: 20 }
        spacing: 12

        Label {
            text: "Your Gut Ranking"
            font.pixelSize: 20
            font.bold: true
            Layout.alignment: Qt.AlignHCenter
        }

        ListView {
            id: rankList
            Layout.fillWidth: true
            Layout.fillHeight: true
            model: sessionController.gutRanking
            spacing: 6
            clip: true
            delegate: Row {
                spacing: 12
                width: rankList.width
                Label {
                    text: "#" + (modelData.rank + 1)
                    font.pixelSize: 18
                    font.bold: true
                    color: Material.accent
                    width: 40
                }
                Label {
                    text: modelData.label
                    font.pixelSize: 18
                }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "Define Criteria  →"
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: sessionController.goToPhase(3)
        }
    }
}
