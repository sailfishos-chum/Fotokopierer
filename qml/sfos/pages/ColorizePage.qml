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

import ".."
import "../../common"

Dialog {
    id: page

    property ScanImage scanImage

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

    FilterImage {
        id: image

        image: scanImage
        filterType: ScanImage.Colorize

        anchors.top: header.bottom
        anchors.bottom: buttons.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * Theme.iconSizeSmall
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
                onClicked: { image.filter.colorMode = ColorizeFilter.BlackAndWhite }
            }
            Button {
                text: "Gray"
                Layout.fillWidth: true
                onClicked: { image.filter.colorMode = ColorizeFilter.Gray }
            }
            Button {
                text: "Color"
                Layout.fillWidth: true
                onClicked: { image.filter.colorMode = ColorizeFilter.FullColor }
            }
            Button {
                text: "Magic"
                Layout.fillWidth: true
                onClicked: { image.filter.colorMode = ColorizeFilter.Colored }
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
        height: closeButton.height + contrast_slider.height * 3
        dock: Dock.Bottom

        Column {
            anchors.left: parent.left
            anchors.right: parent.right

            IconButton {
                id: closeButton
                icon.source: "image://theme/icon-m-dismiss"
                anchors.right: parent.right
                onClicked: { sliders.open = false; buttons.open = true }
            }

            ValueSlider {
                id: contrast_slider
                icon: Qt.resolvedUrl("/icons/contrast.svg")
                onValueChanged: image.filter.contrast = value / 100
            }

            ValueSlider {
                id: brightness_slider
                icon: Qt.resolvedUrl("/icons/brightness.svg")
                onValueChanged: image.filter.brightness = value / 100
            }

            ValueSlider {
                id: details_slider
                icon: Qt.resolvedUrl("image://theme/icon-m-search")
                onValueChanged: image.filter.details = value / 100
            }
        }
    }
}
