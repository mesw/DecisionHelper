import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Drawer {
    id: root
    edge: Qt.BottomEdge
    height: 220
    width: parent ? parent.width : 400

    property string propertyLabel: ""
    property real   rho: 0
    property real   tau: 0
    property real   sigma: 0
    property int    n: 0
    property int    role: 4 // Noise

    function roleLabel(r) {
        switch(r) {
        case 0: return "Dominant Driver"
        case 1: return "Hidden Driver"
        case 2: return "Overrated"
        case 3: return "Inconsistency"
        default: return "Noise"
        }
    }
    function roleColor(r) {
        switch(r) {
        case 0: return "#4caf50"
        case 1: return "#ff9800"
        case 2: return "#f44336"
        case 3: return "#b71c1c"
        default: return "#9e9e9e"
        }
    }

    Column {
        anchors { fill: parent; margins: 20 }
        spacing: 8

        Label {
            text: root.propertyLabel
            font.pixelSize: 20
            font.bold: true
        }
        Rectangle {
            width: parent.width; height: 2
            color: root.roleColor(root.role)
        }
        Label {
            text: root.roleLabel(root.role)
            font.pixelSize: 15
            color: root.roleColor(root.role)
        }
        Row {
            spacing: 24
            Label { text: "ρ = " + root.rho.toFixed(2); font.pixelSize: 14 }
            Label { text: "τ = " + root.tau.toFixed(2); font.pixelSize: 14 }
            Label { text: "n = " + root.n;              font.pixelSize: 14 }
            Label { text: "σ = " + root.sigma.toFixed(2); font.pixelSize: 14 }
        }
        Label {
            wrapMode: Text.WordWrap
            width: parent.width
            font.pixelSize: 12
            color: "#555"
            text: root.rho > 0.6 ? "Strong positive predictor of your gut ranking." :
                  root.rho < -0.4 ? "Inconsistency — favours lower-ranked options." :
                  root.rho > 0.2  ? "Moderate influence on your ranking." :
                                     "Weak or negligible signal."
        }
    }
}
