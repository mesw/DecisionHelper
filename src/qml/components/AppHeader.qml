import QtQuick
import QtQuick.Controls.Material

ToolBar {
    property string title: ""
    property string subtitle: ""

    Material.background: Material.Blue

    Column {
        anchors.centerIn: parent
        spacing: 2
        Label {
            text: title
            font.pixelSize: 18
            font.bold: true
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: subtitle
            font.pixelSize: 12
            color: "#dddddd"
            anchors.horizontalCenter: parent.horizontalCenter
            visible: subtitle.length > 0
        }
    }
}
