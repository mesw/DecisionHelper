import QtQuick
import QtQuick.Controls.Material

Item {
    id: root
    property real value: 0.0       // 0..1 current progress
    property real sweetSpot: 0.7   // fraction where sweet spot tick appears
    height: 24

    ProgressBar {
        id: bar
        anchors.fill: parent
        anchors.bottomMargin: 8
        from: 0; to: 1
        value: root.value
    }

    // Sweet spot tick mark
    Rectangle {
        x: bar.x + bar.width * root.sweetSpot - 1
        y: bar.y
        width: 3
        height: bar.height
        color: Material.accent
        opacity: 0.8
    }

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        font.pixelSize: 10
        text: Math.round(root.value * 100) + "%"
        color: Material.foreground
    }
}
