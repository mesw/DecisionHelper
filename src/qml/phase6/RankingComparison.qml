import QtQuick
import QtQuick.Controls.Material

Item {
    id: root
    implicitHeight: Math.max(gutCol.implicitHeight, valCol.implicitHeight) + 60

    property var data: sessionController.valuesRankingData

    // Column headers
    Row {
        id: headers
        anchors { top: parent.top; left: parent.left; right: parent.right }
        Label { text: "Gut Ranking";    width: parent.width / 2; horizontalAlignment: Text.AlignHCenter; font.bold: true }
        Label { text: "Values Ranking"; width: parent.width / 2; horizontalAlignment: Text.AlignHCenter; font.bold: true }
    }

    // Canvas for connecting lines
    Canvas {
        id: canvas
        anchors { fill: parent; topMargin: 30 }
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = Material.accent
            ctx.lineWidth = 1.5
            ctx.setLineDash([4, 4])

            for (var i = 0; i < root.data.length; ++i) {
                var d = root.data[i]
                // gut position
                var gy = (d.gutRank + 0.5) * rowHeight
                // values position
                var vy = (d.valuesRank + 0.5) * rowHeight
                var mag = Math.abs(d.gutRank - d.valuesRank)
                ctx.lineWidth = 1 + mag * 0.5
                ctx.beginPath()
                ctx.moveTo(width * 0.48, gy)
                ctx.lineTo(width * 0.52, vy)
                ctx.stroke()
            }
        }
        property real rowHeight: root.data.length > 0 ? height / root.data.length : 40
    }

    // Gut ranking column
    Column {
        id: gutCol
        anchors { top: headers.bottom; left: parent.left; topMargin: 4 }
        width: parent.width / 2 - 8
        spacing: 4

        Repeater {
            model: {
                // sort by gutRank
                var arr = []
                for (var i = 0; i < root.data.length; ++i) arr.push(root.data[i])
                arr.sort(function(a,b){ return a.gutRank - b.gutRank })
                return arr
            }
            delegate: Rectangle {
                width: gutCol.width; height: 36; radius: 6
                color: Material.color(Material.Blue, Material.Shade100)
                border.color: Material.accent; border.width: 1
                Label {
                    anchors.centerIn: parent
                    text: "#" + (modelData.gutRank + 1) + " " + modelData.label
                    font.pixelSize: 13
                }
                Behavior on y { NumberAnimation { duration: 400; easing.type: Easing.InOutCubic } }
            }
        }
    }

    // Values ranking column
    Column {
        id: valCol
        anchors { top: headers.bottom; right: parent.right; topMargin: 4 }
        width: parent.width / 2 - 8
        spacing: 4

        Repeater {
            model: {
                var arr = []
                for (var i = 0; i < root.data.length; ++i) arr.push(root.data[i])
                arr.sort(function(a,b){ return a.valuesRank - b.valuesRank })
                return arr
            }
            delegate: Rectangle {
                width: valCol.width; height: 36; radius: 6
                color: Material.color(Material.Green, Material.Shade100)
                border.color: "#4caf50"; border.width: 1
                Label {
                    anchors.centerIn: parent
                    text: "#" + (modelData.valuesRank + 1) + " " + modelData.label
                    font.pixelSize: 13
                }
                Behavior on y { NumberAnimation { duration: 400; easing.type: Easing.InOutCubic } }
            }
        }
    }

    Connections {
        target: sessionController
        function onAnalysisChanged() { canvas.requestPaint() }
    }
}
