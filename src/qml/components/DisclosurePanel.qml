import QtQuick
import QtQuick.Controls.Material

Column {
    id: root
    property string label: "Show details"
    default property alias content: inner.data
    spacing: 0

    Button {
        flat: true
        text: (inner.visible ? "▲ " : "▾ ") + root.label
        onClicked: inner.visible = !inner.visible
        font.pixelSize: 13
        width: parent.width
    }

    Column {
        id: inner
        visible: false
        width: parent.width
        // children injected via default alias
    }
}
