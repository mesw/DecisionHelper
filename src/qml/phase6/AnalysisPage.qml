import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts
import DecisionHelper

Page {
    header: AppHeader {
        title: "Analysis"
        subtitle: "What's really driving your decision?"
    }

    ScrollView {
        anchors.fill: parent
        contentWidth: availableWidth

        ColumnLayout {
            width: parent.width
            spacing: 20
            anchors { left: parent.left; right: parent.right; margins: 16 }
            anchors.topMargin: 16

            // K5: Insight narrative
            InsightCard {
                Layout.fillWidth: true
            }

            // TODO v2: plots disabled for first release
            // Label {
            //     text: "Gut vs Values Ranking"
            //     font.pixelSize: 16; font.bold: true
            // }
            // RankingComparison { Layout.fillWidth: true }   // K2

            // Label {
            //     text: "Property Influence (Spearman ρ)"
            //     font.pixelSize: 16; font.bold: true
            // }
            // CorrelationChart { Layout.fillWidth: true }    // K3

            // Label {
            //     text: "Declared vs Actual Priority"
            //     font.pixelSize: 16; font.bold: true
            // }
            // ScatterPlot { Layout.fillWidth: true }         // K1

            // K4: Coverage disclosure
            DisclosurePanel {
                Layout.fillWidth: true
                label: "Show data coverage"

                ListView {
                    width: parent.width
                    height: contentHeight
                    model: sessionController.correlationData
                    delegate: Row {
                        spacing: 8
                        Label {
                            text: modelData.label
                            width: 140; font.pixelSize: 12
                        }
                        Label {
                            text: "ρ=" + modelData.rho.toFixed(2) + "  n=" + modelData.n
                            font.pixelSize: 12; color: Material.color(Material.Grey)
                        }
                    }
                }
            }

            Row {
                Layout.fillWidth: true
                spacing: 12
                Button {
                    text: "Add Criteria"
                    flat: true
                    onClicked: sessionController.beginAddPropertiesLoop()
                }
                TextField {
                    id: newOptionField
                    placeholderText: "Add option…"
                    width: 160
                    onAccepted: insertOption()
                }
                Button {
                    text: "Insert"
                    Material.background: Material.Blue; Material.foreground: "white"
                    onClicked: insertOption()
                }
            }

            Item { height: 32 } // bottom padding
        }
    }

    function insertOption() {
        if (newOptionField.text.trim().length > 0) {
            sessionController.beginAddOptionInsertion(newOptionField.text.trim())
            newOptionField.text = ""
        }
    }
}
