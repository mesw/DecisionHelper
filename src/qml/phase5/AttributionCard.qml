import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

Rectangle {
    id: card
    radius: 12
    color: Material.background
    border.color: Material.accent
    border.width: 1
    implicitHeight: col.implicitHeight + 24

    signal submitted(string pairKey, var values)

    // Derive pair key from attribution card state
    readonly property string pairKey: {
        var sliders = sessionController.attributionSliders
        if (!sliders || sliders.length === 0) return ""
        // The pairKey comes from the attribution engine; we get the labels from the controller
        // We'll pass the key from AttributionPage which reads it from the controller
        return ""
    }

    property var savedValues: ({})

    Column {
        id: col
        anchors { fill: parent; margins: 12 }
        spacing: 12

        Label {
            text: sessionController.attributionPairLabel
            font.pixelSize: 18
            font.bold: true
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Label {
            text: sessionController.attributionRationale
            font.pixelSize: 12
            color: Material.color(Material.Grey)
            wrapMode: Text.WordWrap
            width: parent.width
        }

        Repeater {
            id: sliderRep
            model: sessionController.attributionSliders

            Column {
                id: sliderCol
                width: col.width
                spacing: 4
                property real sliderVal: sliderItem.value

                Row {
                    width: parent.width
                    Label {
                        text: modelData.propertyLabel
                        font.pixelSize: 14; font.bold: true
                        width: parent.width * 0.55
                    }
                    Label {
                        text: sliderItem.value > 0.05 ? "← better" :
                              sliderItem.value < -0.05 ? "better →" : "equal"
                        font.pixelSize: 12; color: Material.accent
                        width: parent.width * 0.45
                        horizontalAlignment: Text.AlignRight
                    }
                }

                Row {
                    spacing: 4; width: parent.width
                    Label {
                        text: sessionController.attributionPairLeftLabel
                        font.pixelSize: 11; width: 60; wrapMode: Text.WordWrap
                    }
                    Slider {
                        id: sliderItem
                        from: -1; to: 1; stepSize: 0.05
                        value: modelData.value
                        width: parent.width - 128
                    }
                    Label {
                        text: sessionController.attributionPairRightLabel
                        font.pixelSize: 11; width: 60; wrapMode: Text.WordWrap
                        horizontalAlignment: Text.AlignRight
                    }
                }
            }
        }

        Button {
            text: "Submit"
            anchors.horizontalCenter: parent.horizontalCenter
            Material.background: Material.Blue
            Material.foreground: "white"
            onClicked: {
                var vals = {}
                var sliders = sessionController.attributionSliders
                for (var i = 0; i < sliderRep.count && i < sliders.length; ++i) {
                    var colItem = sliderRep.itemAt(i)
                    if (colItem) vals[sliders[i].propertyId] = colItem.sliderVal
                }
                card.submitted(sessionController.attributionPairKey, vals)
            }
        }
    }
}
