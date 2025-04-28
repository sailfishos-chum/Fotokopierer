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

import QtQuick 2.2
import Sailfish.Silica 1.0
import Fotokopierer 1.0

import "../../common"

Page {
    id: page

    property alias source : colimg.source
    property string colormode: "bw"
    property int contrast: contrast_slider.value
    property int brightness: brightness_slider.value
    property int details: details_slider.value

    MouseArea {
        anchors.fill: parent
        onClicked: {
            console.log("BLAAA")
            sliders.open = false
            buttons.open = true
        }
    }

    PageHeader {
        title: qsTr("Colorize")
    }

    ColorizeImage {
        id: colimg

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: buttons.top

        contrast: contrast_slider.value / 100.0
        brightness: brightness_slider.value / 100.0
        details: details_slider.value / 100.0
    }

    DockedPanel {
        id: buttons
        open: true

        width: parent.width
        height: Theme.iconSizeLarge
        dock: Dock.Bottom

        SilicaGridView {
            id: grid

            anchors.fill: parent

            ListModel {
                id: listModel

                ListElement {
                    text: "B/W"
                    name: "bw"
                }

                ListElement {
                    text: "Gray"
                    name: "gray"
                }

                ListElement {
                    text: "Magic"
                    name: "col"
                }

                ListElement {
                    text: "Ctrl"
                    name: "ctrl"
                }

                property var actions : {
                    "bw": function () { colormode = "bw" },
                    "gray": function () { colormode = "gray" },
                    "col": function () { colormode = "colored" },
                    "ctrl": function () {
                        sliders.open = !sliders.open
                        buttons.open = !buttons.open
                    },
                }
            }

            model: listModel

            cellWidth: grid.width / 4
            cellHeight: grid.height

            delegate: Button {
                width: grid.cellWidth
                height: grid.cellHeight
                text: model.text
                onClicked: listModel.actions[name]()
            }
        }
    }

    DockedPanel {
        id: sliders

        width: parent.width
        height: Theme.itemSizeLarge * 3 + Theme.paddingLarge
        dock: Dock.Bottom

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            Slider {
                id: contrast_slider
                anchors.left: parent.left
                anchors.right: parent.right
                minimumValue: 0
                maximumValue: 100
            }

            Slider {
                id: brightness_slider
                anchors.left: parent.left
                anchors.right: parent.right
                minimumValue: 0
                maximumValue: 100
            }

            Slider {
                id: details_slider
                anchors.left: parent.left
                anchors.right: parent.right
                minimumValue: 0
                maximumValue: 100
            }
        }
    }
}
