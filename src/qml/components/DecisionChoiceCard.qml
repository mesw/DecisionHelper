import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Item {
    id: root

    property string title: ""
    property string subtitle: ""
    property color  accentColor: Material.accent
    property bool   isHighlighted: false
    property bool   isSelected: false

    signal clicked()

    Layout.fillWidth: true
    Layout.fillHeight: true

    Rectangle {
        id: cardRect
        anchors.fill: parent
        radius: 12
        color: root.isSelected
            ? Material.color(Material.Blue, Material.Shade50)
            : (mouseArea.pressed
                ? Material.color(Material.Blue, Material.Shade100)
                : (mouseArea.containsMouse ? Material.color(Material.Grey, Material.Shade100) : Material.background))
        border.color: root.isSelected
            ? root.accentColor
            : (root.isHighlighted ? root.accentColor : Material.color(Material.Grey, Material.Shade300))
        border.width: root.isSelected ? 3 : (root.isHighlighted ? 2 : 1)

        Behavior on color {
            ColorAnimation { duration: 120 }
        }

        ColumnLayout {
            anchors.centerIn: parent
            width: parent.width - 24
            spacing: 6

            Label {
                text: root.title
                font.pixelSize: 20
                font.bold: true
                color: root.isSelected ? root.accentColor : Material.foreground
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            Label {
                text: root.isSelected ? "✓ Selected" : root.subtitle
                font.pixelSize: 13
                font.bold: root.isSelected
                color: root.isSelected ? root.accentColor : Material.color(Material.Grey)
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                visible: (root.isSelected || root.subtitle.length > 0)
                Layout.fillWidth: true
            }
        }

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            cursorShape: Qt.PointingHandCursor
            onClicked: root.clicked()
        }
    }
}
