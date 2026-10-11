import QtQuick
import QtQuick.Controls.Material
import DecisionHelper

Item {
    id: root
    implicitHeight: 280
    property var data: sessionController.scatterData

    function roleColor(role) {
        switch (role) {
        case 0: return "#4caf50"   // DominantDriver
        case 1: return "#ff9800"   // HiddenDriver
        case 2: return "#f44336"   // Overrated
        case 3: return "#b71c1c"   // Inconsistency
        default: return "#9e9e9e"  // Noise
        }
    }

    // chart margins
    readonly property real mL: 48
    readonly property real mR: 16
    readonly property real mT: 20
    readonly property real mB: 40
    readonly property real cW: Math.max(1, width  - mL - mR)
    readonly property real cH: Math.max(1, height - mT - mB)

    // Map declared priority (1-based) to x pixel
    function xPos(priority) {
        var n = Math.max(data.length, 1)
        return mL + cW * priority / (n + 1)
    }
    // Map ρ ∈ [-1, 1] to y pixel (high ρ → top)
    function yPos(rho) {
        return mT + (1.0 - rho) / 2.0 * cH
    }

    PropertyDetailSheet { id: detailSheet }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var n = root.data.length

            // y-axis
            ctx.strokeStyle = "#999"
            ctx.lineWidth = 1
            ctx.beginPath()
            ctx.moveTo(root.mL, root.mT)
            ctx.lineTo(root.mL, root.mT + root.cH)
            ctx.lineTo(root.mL + root.cW, root.mT + root.cH)
            ctx.stroke()

            // ρ = 0 guide
            var y0 = root.yPos(0)
            ctx.strokeStyle = "#ddd"
            ctx.setLineDash([4, 4])
            ctx.beginPath()
            ctx.moveTo(root.mL, y0)
            ctx.lineTo(root.mL + root.cW, y0)
            ctx.stroke()
            ctx.setLineDash([])

            // y-axis tick labels
            ctx.fillStyle = "#666"
            ctx.font = "11px sans-serif"
            ctx.textAlign = "right"
            ctx.textBaseline = "middle"
            ctx.fillText("+1", root.mL - 6, root.yPos(1))
            ctx.fillText("0",  root.mL - 6, root.yPos(0))
            ctx.fillText("-1", root.mL - 6, root.yPos(-1))

            // y-axis title (rotated)
            ctx.save()
            ctx.translate(12, root.mT + root.cH / 2)
            ctx.rotate(-Math.PI / 2)
            ctx.textAlign = "center"
            ctx.textBaseline = "middle"
            ctx.fillText("ρ (actual)", 0, 0)
            ctx.restore()

            // x-axis labels
            ctx.textAlign = "center"
            ctx.textBaseline = "top"
            var axisY = root.mT + root.cH + 5
            for (var xi = 0; xi < n; ++xi) {
                var dxi = root.data[xi]
                var p   = dxi.declaredPriority > 0 ? dxi.declaredPriority : xi + 1
                ctx.fillText(String(p), root.xPos(p), axisY)
            }
            ctx.fillText("Declared Priority →", root.mL + root.cW / 2, axisY + 16)

            if (n === 0) {
                ctx.fillStyle = "#aaa"
                ctx.textAlign = "center"
                ctx.textBaseline = "middle"
                ctx.fillText("No data yet.", root.mL + root.cW / 2, root.mT + root.cH / 2)
                return
            }

            // dots + labels
            var radius = 7
            for (var i = 0; i < n; ++i) {
                var d  = root.data[i]
                var p2 = d.declaredPriority > 0 ? d.declaredPriority : i + 1
                var px = root.xPos(p2)
                var py = root.yPos(d.rho)

                ctx.fillStyle = roleColor(d.role)
                ctx.beginPath()
                ctx.arc(px, py, radius, 0, 2 * Math.PI)
                ctx.fill()

                ctx.fillStyle = "#333"
                ctx.font = "10px sans-serif"
                ctx.textAlign = "center"
                ctx.textBaseline = "bottom"
                ctx.fillText(d.label, px, py - radius - 2)
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: function(mouse) {
            var hitR2 = 196  // 14px hit radius squared
            for (var i = 0; i < root.data.length; ++i) {
                var d  = root.data[i]
                var p  = d.declaredPriority > 0 ? d.declaredPriority : i + 1
                var px = root.xPos(p)
                var py = root.yPos(d.rho)
                var dx = mouse.x - px
                var dy = mouse.y - py
                if (dx*dx + dy*dy < hitR2) {
                    detailSheet.propertyLabel = d.label
                    detailSheet.rho           = d.rho
                    detailSheet.tau           = d.tau
                    detailSheet.sigma         = d.stdDev
                    detailSheet.n             = d.n
                    detailSheet.role          = d.role
                    detailSheet.open()
                    return
                }
            }
        }
    }

    Connections {
        target: sessionController
        function onAnalysisChanged() { canvas.requestPaint() }
    }
    onWidthChanged: canvas.requestPaint()
    onDataChanged:  canvas.requestPaint()
}
