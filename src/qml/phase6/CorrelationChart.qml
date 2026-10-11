import QtQuick
import QtQuick.Controls.Material

Item {
    id: root
    property var data: sessionController.correlationData
    implicitHeight: Math.max(80, data.length * 36 + 56)

    function roleColor(role) {
        switch (role) {
        case 0: return "#4caf50"   // DominantDriver
        case 1: return "#ff9800"   // HiddenDriver
        case 2: return "#f44336"   // Overrated
        case 3: return "#b71c1c"   // Inconsistency
        default: return "#9e9e9e"  // Noise
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent

        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)

            var n = root.data.length
            if (n === 0) {
                ctx.fillStyle = "#aaa"
                ctx.font = "13px sans-serif"
                ctx.textAlign = "left"
                ctx.textBaseline = "top"
                ctx.fillText("No property data yet.", 16, 16)
                return
            }

            var labelW  = 130
            var marginT = 8
            var marginB = 36
            var rowH    = 32
            var chartX  = labelW
            var chartW  = width - labelW - 16
            var zeroX   = chartX + chartW / 2

            // vertical grid lines at -1, -0.5, 0, +0.5, +1
            var gridTicks = [-1, -0.5, 0, 0.5, 1]
            for (var t = 0; t < gridTicks.length; ++t) {
                var tx = zeroX + gridTicks[t] * (chartW / 2)
                ctx.strokeStyle = gridTicks[t] === 0 ? "#bbb" : "#eee"
                ctx.lineWidth = gridTicks[t] === 0 ? 1.5 : 1
                ctx.beginPath()
                ctx.moveTo(tx, marginT)
                ctx.lineTo(tx, marginT + n * rowH)
                ctx.stroke()
            }

            // x-axis labels
            ctx.fillStyle = "#888"
            ctx.font = "11px sans-serif"
            ctx.textAlign = "center"
            ctx.textBaseline = "top"
            var axisY = marginT + n * rowH + 5
            ctx.fillText("-1",  chartX,              axisY)
            ctx.fillText("-½",  zeroX - chartW / 4,  axisY)
            ctx.fillText("0",   zeroX,                axisY)
            ctx.fillText("+½",  zeroX + chartW / 4,  axisY)
            ctx.fillText("+1",  chartX + chartW,      axisY)
            ctx.fillText("Spearman ρ", chartX + chartW / 2, axisY + 16)

            // bars
            for (var i = 0; i < n; ++i) {
                var d    = root.data[i]
                var y    = marginT + i * rowH
                var barH = rowH - 6
                var barY = y + 3

                // row label
                ctx.fillStyle = "#333"
                ctx.font = "13px sans-serif"
                ctx.textAlign = "right"
                ctx.textBaseline = "middle"
                ctx.fillText(d.label, labelW - 8, barY + barH / 2)

                // bar
                var rho  = Math.max(-1, Math.min(1, d.rho))
                var barW = Math.abs(rho) * (chartW / 2)
                var barX = rho >= 0 ? zeroX : zeroX - barW
                ctx.fillStyle = roleColor(d.role)
                ctx.fillRect(barX, barY, Math.max(barW, 1), barH)

                // value label
                ctx.font = "11px sans-serif"
                ctx.textBaseline = "middle"
                if (barW > 32) {
                    ctx.fillStyle = "white"
                    ctx.textAlign = rho >= 0 ? "left" : "right"
                    ctx.fillText(rho.toFixed(2), rho >= 0 ? barX + 4 : barX + barW - 4, barY + barH / 2)
                } else {
                    ctx.fillStyle = "#555"
                    ctx.textAlign = rho >= 0 ? "left" : "right"
                    ctx.fillText(rho.toFixed(2), rho >= 0 ? zeroX + barW + 4 : zeroX - barW - 4, barY + barH / 2)
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
