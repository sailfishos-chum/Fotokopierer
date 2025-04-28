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

    property alias image: cutview.scanImage

    canNavigateForward: cutview.valid

    onStatusChanged: {
        if (status == PageStatus.Deactivating) {
            cutview.cutImage()
        }
    }

    PageHeader {
        id: header
        title: qsTr("Cut & Rotate")
    }

    CutImageView {
        id: cutview

        anchors.top: header.bottom
        anchors.bottom: buttons.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * Theme.iconSizeSmall
        markerColor: Theme.primaryColor
        lineColor: Theme.highlightColor
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
                    icon: "image://theme/icon-m-rotate-left"
                    name: "left"
                }

                ListElement {
                    icon: "image://theme/icon-m-rotate-right"
                    name: "right"
                }

                ListElement {
                    // icon: "image://theme/icon-m-crop"
                    text: "auto"
                    name: "auto"
                }

                ListElement {
                    // icon: "image://theme/icon-m-display"
                    text: "max"
                    name: "all"
                }

                property var actions : {
                    "left": function () { cutview.rotateLeft() },
                    "right": function () { cutview.rotateRight() },
                    "auto": function () { cutview.selectAuto() },
                    "all": function () { cutview.selectAll() },
                }
            }

            model: listModel

            cellWidth: grid.width / 4
            cellHeight: grid.height

            delegate: Item {
                width: grid.cellWidth
                height: grid.cellHeight

                Button {
                    visible: model.text ? true : false
                    anchors.fill: parent
                    text: model.text || ""
                    onClicked: listModel.actions[name]()
                }

                IconButton {
                    visible: model.icon ? true : false
                    anchors.fill: parent
                    icon.source: model.icon || ""
                    onClicked: listModel.actions[name]()
                }
            }
        }
    }
}
