import QtQuick
import QtQuick.Controls.Material

StackView {
    id: stack

    Connections {
        target: sessionController
        function onCurrentPhaseChanged() {
            var phase = sessionController.currentPhase
            if (phase === 1) {
                stack.replace(null, Qt.resolvedUrl("phase1/NameDecisionPage.qml"))
            } else if (phase === 2) {
                stack.replace(null, Qt.resolvedUrl("phase2/AddOptionsPage.qml"))
            } else if (phase === 3) {
                stack.replace(null, Qt.resolvedUrl("phase3/DefineCriteriaPage.qml"))
            } else if (phase === 4) {
                stack.replace(null, Qt.resolvedUrl("phase4/OptionMicroDecisionPage.qml"))
            } else if (phase === 5) {
                stack.replace(null, Qt.resolvedUrl("phase5/CriteriaWeightingPage.qml"))
            } else if (phase === 6) {
                stack.replace(null, Qt.resolvedUrl("phase6/ResultsPage.qml"))
            }
        }
    }

    Component.onCompleted: {
        stack.push(Qt.resolvedUrl("phase1/NameDecisionPage.qml"))
    }
}
