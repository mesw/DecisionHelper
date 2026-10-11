import QtQuick
import QtQuick.Controls.Material

Rectangle {
    id: card
    property string optionLabel: ""
    property bool   isLeft: true  // left/top vs right/bottom

    signal chosen()
    signal tied()

    radius: 16
    color: Material.background
    border.color: Material.accent
    border.width: 2

    // Shadow
    layer.enabled: true
    layer.effect: null

    Column {
        anchors.centerIn: parent
        spacing: 8

        Label {
            text: card.optionLabel
            font.pixelSize: 22
            font.bold: true
            horizontalAlignment: Text.AlignHCenter
            anchors.horizontalCenter: parent.horizontalCenter
            wrapMode: Text.WordWrap
            width: card.width - 32
        }

        Button {
            text: "Choose"
            anchors.horizontalCenter: parent.horizontalCenter
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: card.chosen()
        }
    }

    // Swipe/drag support
    DragHandler {
        id: drag
        property real startX: 0
        property real startY: 0

        onActiveChanged: {
            if (!active) {
                var dx = card.x - startX
                var dy = card.y - startY
                card.x = startX
                card.y = startY
                var threshold = Math.min(card.width, card.height) * 0.3
                if (Math.abs(dx) > threshold || Math.abs(dy) > threshold)
                    card.chosen()
            } else {
                startX = card.x
                startY = card.y
            }
        }
    }
}
