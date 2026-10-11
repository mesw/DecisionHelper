import QtQuick
import QtQuick.Controls.Material
import QtQuick.Layouts

ToolBar {
    id: toolbar
    property string title: ""
    property string subtitle: ""

    Material.background: Material.Blue

    function openLicenseDialog() {
        licenseDialog.open()
    }

    Column {
        anchors.centerIn: parent
        spacing: 2
        Label {
            text: title
            font.pixelSize: 18
            font.bold: true
            color: "white"
            anchors.horizontalCenter: parent.horizontalCenter
        }
        Label {
            text: subtitle
            font.pixelSize: 12
            color: "#dddddd"
            anchors.horizontalCenter: parent.horizontalCenter
            visible: subtitle.length > 0
        }
    }

    ToolButton {
        id: licenseBtn
        anchors.right: parent.right
        anchors.rightMargin: 6
        anchors.verticalCenter: parent.verticalCenter
        contentItem: RowLayout {
            spacing: 4
            Label {
                text: "⚖"
                font.pixelSize: 13
                font.family: typeof emojiFontFamily !== "undefined" && emojiFontFamily.length > 0 ? emojiFontFamily : ""
                color: "white"
            }
            Label {
                text: "License Info"
                font.pixelSize: 11
                font.bold: true
                color: "white"
            }
        }
        onClicked: licenseDialog.open()
    }

    Dialog {
        id: licenseDialog
        title: "License & Legal Information"
        modal: true
        parent: Overlay.overlay
        anchors.centerIn: parent
        width: Math.min(parent ? parent.width - 24 : 360, 480)
        standardButtons: Dialog.Close

        Flickable {
            width: parent.width
            implicitHeight: Math.min(contentColumn.implicitHeight, 460)
            contentHeight: contentColumn.implicitHeight
            clip: true
            boundsBehavior: Flickable.StopAtBounds

            ScrollBar.vertical: ScrollBar {
                policy: ScrollBar.AsNeeded
            }

            ColumnLayout {
                id: contentColumn
                width: parent.width - 12
                spacing: 12

                // Application Card
                Rectangle {
                    Layout.fillWidth: true
                    radius: 8
                    color: Material.color(Material.Blue, Material.Shade50)
                    border.color: Material.color(Material.Blue, Material.Shade200)
                    implicitHeight: appCol.implicitHeight + 20

                    ColumnLayout {
                        id: appCol
                        anchors { fill: parent; margins: 10 }
                        spacing: 4

                        RowLayout {
                            spacing: 8
                            Label {
                                text: "DecisionHelper"
                                font.pixelSize: 16
                                font.bold: true
                                color: Material.color(Material.Blue, Material.Shade900)
                            }
                            Label {
                                text: "v1.0"
                                font.pixelSize: 11
                                color: Material.color(Material.Blue, Material.Shade700)
                            }
                        }

                        Label {
                            text: "Author: Michael Egger (MESW)"
                            font.pixelSize: 12
                            font.bold: true
                            color: "#222"
                        }

                        Label {
                            text: "Copyright © 2026 Michael Egger (MESW). All rights reserved."
                            font.pixelSize: 11
                            color: "#444"
                        }

                        Label {
                            text: "License: GNU General Public License v3.0 (GPL-3.0)"
                            font.pixelSize: 12
                            font.bold: true
                            color: Material.color(Material.Blue, Material.Shade900)
                        }

                        Label {
                            text: "This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.\n\nDistributed WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
                            font.pixelSize: 10
                            color: "#555"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }

                // Third Party Section Header
                Label {
                    text: "Third-Party Components & Licenses"
                    font.pixelSize: 13
                    font.bold: true
                    color: Material.color(Material.Grey, Material.Shade800)
                }

                // 1. Qt Framework
                Rectangle {
                    Layout.fillWidth: true
                    radius: 6
                    color: Material.color(Material.Grey, Material.Shade100)
                    border.color: Material.color(Material.Grey, Material.Shade300)
                    implicitHeight: qtCol.implicitHeight + 16

                    ColumnLayout {
                        id: qtCol
                        anchors { fill: parent; margins: 8 }
                        spacing: 2

                        Label {
                            text: "Qt 6 Framework (QtQuick / QML / Core)"
                            font.bold: true
                            font.pixelSize: 12
                        }
                        Label {
                            text: "Copyright © 2024 The Qt Company Ltd. and other contributors."
                            font.pixelSize: 10
                            color: "#555"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                        Label {
                            text: "License: GNU LGPL v3 / GNU GPL v3"
                            font.pixelSize: 11
                            color: Material.color(Material.Blue, Material.Shade800)
                        }
                        Label {
                            text: "The Qt GUI Toolkit is used under the GNU Lesser General Public License version 3. Full Qt source code and licensing terms: https://www.qt.io/licensing/"
                            font.pixelSize: 10
                            color: "#666"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }

                // 2. Roboto Font
                Rectangle {
                    Layout.fillWidth: true
                    radius: 6
                    color: Material.color(Material.Grey, Material.Shade100)
                    border.color: Material.color(Material.Grey, Material.Shade300)
                    implicitHeight: robotoCol.implicitHeight + 16

                    ColumnLayout {
                        id: robotoCol
                        anchors { fill: parent; margins: 8 }
                        spacing: 2

                        Label {
                            text: "Roboto Font"
                            font.bold: true
                            font.pixelSize: 12
                        }
                        Label {
                            text: "Copyright © 2015 Google LLC"
                            font.pixelSize: 10
                            color: "#555"
                        }
                        Label {
                            text: "License: Apache License, Version 2.0"
                            font.pixelSize: 11
                            color: Material.color(Material.Blue, Material.Shade800)
                        }
                        Label {
                            text: "Licensed under the Apache License, Version 2.0. You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0"
                            font.pixelSize: 10
                            color: "#666"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }

                // 3. Noto Color Emoji Font
                Rectangle {
                    Layout.fillWidth: true
                    radius: 6
                    color: Material.color(Material.Grey, Material.Shade100)
                    border.color: Material.color(Material.Grey, Material.Shade300)
                    implicitHeight: notoCol.implicitHeight + 16

                    ColumnLayout {
                        id: notoCol
                        anchors { fill: parent; margins: 8 }
                        spacing: 2

                        Label {
                            text: "Google Noto Color Emoji Font"
                            font.bold: true
                            font.pixelSize: 12
                        }
                        Label {
                            text: "Copyright © 2013-2024 Google LLC"
                            font.pixelSize: 10
                            color: "#555"
                        }
                        Label {
                            text: "License: SIL Open Font License, Version 1.1 (OFL-1.1)"
                            font.pixelSize: 11
                            color: Material.color(Material.Blue, Material.Shade800)
                        }
                        Label {
                            text: "Licensed under the SIL Open Font License, Version 1.1. Available with a FAQ at http://scripts.sil.org/OFL"
                            font.pixelSize: 10
                            color: "#666"
                            wrapMode: Text.WordWrap
                            Layout.fillWidth: true
                        }
                    }
                }
            }
        }
    }
}
