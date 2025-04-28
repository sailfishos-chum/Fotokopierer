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

import "../../common"

Page {
    id: page

    PageHeader {
        title: qsTr("Cut & Rotate")
    }

    CutImage {
        id: img
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: buttons.top
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

                property var actions : {
                    "left": function () { img.rotateLeft() },
                    "right": function () { img.rotateRight() }
                }
            }

            model: listModel

            cellWidth: grid.width / 2
            cellHeight: grid.height

            delegate: IconButton {
                width: grid.cellWidth
                height: grid.cellHeight
                icon.source: model.icon
                onClicked: listModel.actions[name]()
            }
        }
    }
}
