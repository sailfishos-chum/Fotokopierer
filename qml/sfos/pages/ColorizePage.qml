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
import QtQuick.Layouts 1.0
import Sailfish.Silica 1.0
import Fotokopierer 1.0

import "../../common"

Dialog {
    id: page

    property alias source : colimg.source
    property alias colorMode: colimg.colorMode
    property alias contrast: contrast_slider.value
    property alias brightness: brightness_slider.value
    property alias details: details_slider.value
    property alias image: colimg

    MouseArea {
        anchors.fill: parent
        onClicked: {
            sliders.open = false
            buttons.open = true
        }
    }

    PageHeader {
        id: header
        title: qsTr("Colorize")
    }

    ColorizeImage {
        id: colimg

        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: buttons.top

        contrast: contrast_slider.value / 100.0
        brightness: brightness_slider.value / 100.0
        details: details_slider.value / 100.0
        colorMode: ColorizeImage.BlackAndWhite
    }

    DockedPanel {
        id: buttons
        open: true

        width: parent.width
        height: Theme.iconSizeLarge
        dock: Dock.Bottom

        RowLayout {
            id: buttonRow
            anchors { left: parent.left; right: parent.right }
            Button {
                text: "B/W"
                Layout.fillWidth: true
                onClicked: { colimg.colorMode = ColorizeImage.BlackAndWhite }
            }
            Button {
                text: "Gray"
                Layout.fillWidth: true
                onClicked: { colimg.colorMode = ColorizeImage.Gray }
            }
            Button {
                text: "Magic"
                Layout.fillWidth: true
                onClicked: { colimg.colorMode = ColorizeImage.Colored }
            }
            Button {
                text: "Ctrl"
                Layout.fillWidth: true
                onClicked: { sliders.open = !sliders.open; buttons.open = !buttons.open }
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
                value: 50
                minimumValue: 0
                maximumValue: 100
            }

            Slider {
                id: brightness_slider
                anchors.left: parent.left
                anchors.right: parent.right
                value: 50
                minimumValue: 0
                maximumValue: 100
            }

            Slider {
                id: details_slider
                anchors.left: parent.left
                anchors.right: parent.right
                value: 50
                minimumValue: 0
                maximumValue: 100
            }
        }
    }
}
