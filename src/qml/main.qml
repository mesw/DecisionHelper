import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ApplicationWindow {
    id: root
    visible: true
    width: 400
    height: 700
    title: "DecisionHelper"

    Material.theme: Material.Light
    Material.accent: Material.Blue

    Component.onCompleted: {
        if (sessionController.hasSavedSession) {
            resumeDialog.open()
        }
    }

    PhaseRouter {
        id: router
        anchors.fill: parent
    }

    Dialog {
        id: resumeDialog
        title: "Resume session?"
        anchors.centerIn: parent
        modal: true
        standardButtons: Dialog.Yes | Dialog.No
        Label {
            text: "A previous session was found. Resume it?"
            wrapMode: Text.WordWrap
            width: parent.width
        }
        onAccepted: sessionController.loadSession()
        onRejected: sessionController.newSession()
    }
}
