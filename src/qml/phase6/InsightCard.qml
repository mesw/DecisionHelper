import QtQuick
import QtQuick.Controls.Material

Rectangle {
    id: card
    radius: 12
    color: "#e3f2fd"
    border.color: Material.accent
    border.width: 1
    implicitHeight: label.implicitHeight + 32

    Label {
        id: label
        anchors { fill: parent; margins: 16 }
        text: sessionController.insightNarrative
        wrapMode: Text.WordWrap
        font.pixelSize: 14
        textFormat: Text.RichText
        lineHeight: 1.4
    }
}
