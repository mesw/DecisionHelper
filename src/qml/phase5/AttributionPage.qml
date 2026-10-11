import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    header: AppHeader {
        title: "Rate Properties"
        subtitle: "For each pair, how much better is one vs the other?"
    }

    ColumnLayout {
        anchors { fill: parent; margins: 16 }
        spacing: 12

        SweetSpotProgressBar {
            Layout.fillWidth: true
            value: sessionController.attributionProgress
        }

        // Sweet-spot nudge popup
        Popup {
            id: sweetSpotPopup
            anchors.centerIn: parent
            modal: false
            focus: false
            closePolicy: Popup.CloseOnEscape | Popup.CloseOnPressOutside
            width: 280

            Column {
                spacing: 8; padding: 16
                Label {
                    text: "You've answered enough to see patterns!"
                    font.bold: true
                    wrapMode: Text.WordWrap
                    width: 240
                }
                Label {
                    text: "Keep going for more precision, or head to Analysis."
                    wrapMode: Text.WordWrap
                    width: 240
                }
                Row {
                    spacing: 8
                    Button { text: "Keep going"; flat: true; onClicked: sweetSpotPopup.close() }
                    Button {
                        text: "Analyse now"
                        Material.background: Material.Blue; Material.foreground: "white"
                        onClicked: { sweetSpotPopup.close(); sessionController.beginAnalysis() }
                    }
                }
            }
        }

        Connections {
            target: sessionController
            function onAttributionChanged() {
                if (sessionController.attributionAtSweetSpot && !sweetSpotPopup.visible)
                    sweetSpotPopup.open()
            }
        }

        Loader {
            id: cardLoader
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceComponent: sessionController.attributionDone ? doneComp : cardComp
        }

        Component {
            id: cardComp
            AttributionCard {
                width: cardLoader.width
                onSubmitted: function(pk, vals) {
                    sessionController.submitAttributionCard(pk, vals)
                }
            }
        }

        Component {
            id: doneComp
            Column {
                spacing: 16; anchors.centerIn: parent
                Label { text: "All pairs rated!"; font.pixelSize: 18; anchors.horizontalCenter: parent.horizontalCenter }
            }
        }

        Button {
            Layout.fillWidth: true
            text: "View Analysis  →"
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: sessionController.beginAnalysis()
        }
    }
}
