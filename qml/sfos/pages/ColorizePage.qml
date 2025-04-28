/*
 * Copyright (c) 2018-2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

    onAccepted: colview.apply()

    ColorizeView {
        id: colview

        scanner: Scanner

        anchors.top: header.bottom
        anchors.bottom: buttons.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * Theme.iconSizeSmall

        BusyIndicator {
            size: BusyIndicatorSize.Large
            anchors.centerIn: parent
            running: colview.busy
        }
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
            IconButton {
                icon.source: Qt.resolvedUrl("/icons/icon-m-bw.svg")
                icon.width: Theme.iconSizeMedium
                icon.height: Theme.iconSizeMedium
                icon.color: undefined
                Layout.fillWidth: true
                onClicked: { colview.colorMode = ColorizeView.BlackAndWhite }
            }
            IconButton {
                icon.source: Qt.resolvedUrl("/icons/icon-m-gray.svg")
                icon.width: Theme.iconSizeMedium
                icon.height: Theme.iconSizeMedium
                icon.color: undefined
                Layout.fillWidth: true
                onClicked: { colview.colorMode = ColorizeView.Gray }
            }
            IconButton {
                icon.source: Qt.resolvedUrl("/icons/icon-m-color.svg")
                icon.width: Theme.iconSizeMedium
                icon.height: Theme.iconSizeMedium
                icon.color: undefined
                Layout.fillWidth: true
                onClicked: { colview.colorMode = ColorizeView.FullColor }
            }
            IconButton {
                icon.source: Qt.resolvedUrl("/icons/icon-m-special.svg")
                icon.width: Theme.iconSizeMedium
                icon.height: Theme.iconSizeMedium
                icon.color: undefined
                Layout.fillWidth: true
                onClicked: { colview.colorMode = ColorizeView.Colored }
            }
            IconButton {
                icon.source: Qt.resolvedUrl("/icons/icon-m-ctrl.svg")
                icon.width: Theme.iconSizeMedium
                icon.height: Theme.iconSizeMedium
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
                onValueChanged: colview.contrast = value / 100
            }

            ValueSlider {
                id: brightness_slider
                icon: Qt.resolvedUrl("/icons/brightness.svg")
                onValueChanged: colview.brightness = value / 100
            }

            ValueSlider {
                id: details_slider
                icon: Qt.resolvedUrl("image://theme/icon-m-search")
                onValueChanged: colview.details = value / 100
            }
        }
    }

    function apply() {
        colview.apply()
    }
}
