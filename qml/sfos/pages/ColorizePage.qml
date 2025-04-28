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
            size: BusyIndicatorSize.Small
            anchors.top: parent.top
            anchors.right: parent.right
            running: colview.busy
        }
    }

    DockedPanel {
        id: buttons
        open: true

        width: parent.width
        height: Theme.iconSizeLarge
        dock: Dock.Bottom

        SilicaGridView {
            id: buttonView

            anchors.fill: parent

            ListModel {
                id: listModel

                ListElement {
                    icon: "/icons/icon-m-bw.svg"
                    colorMode: 1  // ColorMode.BlackAndWhite
                }

                ListElement {
                    icon: "/icons/icon-m-gray.svg"
                    colorMode: 0  // ColorMode.Gray
                }

                ListElement {
                    icon: "/icons/icon-m-color.svg"
                    colorMode: 3  // ColorMode.FullColor
                }

                ListElement {
                    icon: "/icons/icon-m-special.svg"
                    colorMode: 2 // ColorMode.Colored
                }

                ListElement {
                    icon: "/icons/icon-m-ctrl.svg"
                    colorMode: -1
                }
            }

            model: listModel

            cellWidth: width / model.count
            cellHeight: height

            delegate: BackgroundItem {
                width: buttonView.cellWidth
                height: buttonView.cellHeight

                onClicked: {
                    if (colorMode == -1) {
                        sliders.open = !sliders.open
                        buttons.open = !buttons.open
                    } else {
                        colview.colorMode = colorMode
                    }
                }

                highlightedColor: Theme.rgba(Theme.highlightBackgroundColor, Theme.highlightBackgroundOpacity)

                Rectangle {
                    anchors.fill: parent
                    color: Theme.rgba(Theme.highlightBackgroundColor, 0.5 * Theme.highlightBackgroundOpacity)
                    radius: 5
                    visible: colview.colorMode == colorMode
                }

                Image {
                    anchors.centerIn: parent
                    sourceSize.width: Theme.iconSizeMedium
                    sourceSize.height: Theme.iconSizeMedium
                    source: Qt.resolvedUrl(icon)
                }
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
                visible: colview.colorMode == ColorizeView.Gray || colview.colorMode == ColorizeView.FullColor
                onValueChanged: colview.contrast = value / 100
            }

            ValueSlider {
                id: brightness_slider
                icon: Qt.resolvedUrl("/icons/brightness.svg")
                visible: contrast_slider.visible
                onValueChanged: colview.brightness = value / 100
            }

            ValueSlider {
                id: threshold_slider
                icon: Qt.resolvedUrl("image://theme/icon-m-search")
                visible: !contrast_slider.visible
                onValueChanged: colview.threshold = value / 100
            }

            ValueSlider {
                id: blocksize_slider
                icon: Qt.resolvedUrl("image://theme/icon-m-search")
                visible: !contrast_slider.visible
                onValueChanged: colview.blockSize = value / 100
            }
        }
    }

    function apply() {
        colview.apply()
    }
}
