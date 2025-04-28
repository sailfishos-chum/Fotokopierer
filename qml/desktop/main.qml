/*
 * Copyright (c) 2018 Frank Fischer <frank-fischer@shadow-soft.de>
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see  <http://www.gnu.org/licenses/>
 */

import QtQuick 2.0
import QtQuick.Controls 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Layouts 1.0
import Fotokopierer 1.0
import "../common"

ApplicationWindow {
    visible: true
    title: "Fotokopierer"

    width: 400
    height: 600

    property string colormode : "colored"

    Component.onCompleted: {
        if (Qt.application.arguments.length > 1) {
            img.loadFile(Qt.application.arguments[1])
            cutimage.img = img
        }
    }

    ScannedImage {
        id: img
    }

    Item {
        id: cutbox;
        anchors.fill: parent

        CutImage {
            id: cutimage

            img: img

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.bottom: cutbuttons.top
        }

        Row {
            id: cutbuttons
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Button {
                text: "Left"
                style: ButtonStyle {
                    background: Rectangle {
                        implicitWidth: 100
                        implicitHeight: 25
                        border.width: control.activeFocus ? 2 : 1
                        border.color: "#888"
                        radius: 4
                        gradient: Gradient {
                            GradientStop { position: 0 ; color: control.pressed ? "#ccc" : "#eee" }
                            GradientStop { position: 1 ; color: control.pressed ? "#aaa" : "#ccc" }
                        }
                    }

                    label: Text {
                        color: "black"
                        horizontalAlignment: Text.AlignHCenter
                        text: control.text
                    }
                }

                onClicked: cutimage.rotateLeft()
            }

            Button {
                text: "Right"
                onClicked: cutimage.rotateRight()
            }

            Button {
                text: "All"
                onClicked: cutimage.selectAll()
            }

            Button {
                text: "Auto"
                onClicked: cutimage.selectAuto()            }

            Button {
                text: "Accept"
                onClicked: {
                    cutbox.visible = false
                    colbox.visible = true
                }
            }
        }
    }

    Item {
        id: colbox
        anchors.fill: parent
        visible: false

        Image {
            id: colimage

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: contrastRow.top

            fillMode: Image.PreserveAspectFit

            source: "image://Scanned/" + img.image + "/cut" +
                "/" + colormode +
                "/" + contrast.value +
                "/" + brightness.value +
                "/" + details.value
            cache: false
        }

        Row {
            id: contrastRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: brightnessRow.top

            Label {
                text: "Contrast: "
            }
            Slider {
                id: contrast
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: 50
            }
        }

        Row {
            id: brightnessRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: detailsRow.top

            Label {
                text: "Brightness"
            }
            Slider {
                id: brightness
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: 50
            }
        }

        Row {
            id: detailsRow
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: colbuttons.top

            Label {
                text: "Details"
            }
            Slider {
                id: details
                minimumValue: 0
                maximumValue: 100
                stepSize: 1
                value: 50
            }
        }

        Row {
            id: colbuttons
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            Button {
                text: "B&W"
                onClicked: colormode = "bw"
            }

            Button {
                text: "Gray"
                onClicked: colormode = "gray"
            }

            Button {
                text: "Colored"
                onClicked: colormode = "colored"
            }

            Button {
                text: "Accept"
                onClicked: {
                    cutbox.visible = true
                    colbox.visible = false
                }
            }
        }
    }
}
