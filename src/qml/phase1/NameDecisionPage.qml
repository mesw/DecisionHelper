import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    id: root

    header: AppHeader {
        title: "Step 1: Decision Name"
        subtitle: "What are you trying to decide?"
    }

    ColumnLayout {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 16

        Item { Layout.fillHeight: true } // Top spacer

        Label {
            text: "Give your decision a title:"
            font.pixelSize: 18
            font.bold: true
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            text: "e.g. 'Choosing a new laptop', 'What to order for dinner', 'Which job offer to take'"
            font.pixelSize: 13
            color: Material.color(Material.Grey)
            wrapMode: Text.WordWrap
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        TextField {
            id: titleField
            placeholderText: "Enter decision title…"
            text: sessionController.decisionTitle
            font.pixelSize: 18
            Layout.fillWidth: true
            Layout.maximumWidth: 420
            Layout.alignment: Qt.AlignHCenter
            onTextEdited: sessionController.setDecisionTitle(titleField.text)
            onAccepted: {
                if (sessionController.canProceedToPhase2) {
                    sessionController.goToPhase(2)
                }
            }
        }

        Item { height: 12 }

        Button {
            text: "Next: Enter Options →"
            font.pixelSize: 16
            Layout.fillWidth: true
            Layout.maximumWidth: 420
            Layout.alignment: Qt.AlignHCenter
            Layout.preferredHeight: 48
            enabled: sessionController.canProceedToPhase2
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: sessionController.goToPhase(2)
        }

        Item { Layout.fillHeight: true } // Bottom spacer
    }
}
