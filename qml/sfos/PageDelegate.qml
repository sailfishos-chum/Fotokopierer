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

import Sailfish.Silica 1.0
import Fotokopierer 1.0
import QtQml.Models 2.2
import QtQuick 2.0

import "../common"

DragDelegate {
    id: dragDelegate

    property var page
    property bool isAddButton: false
    property double factor: 0.9

    dragEnabled: !isAddButton

    signal addPage()

    Component {
        id: addButtonView

        IconButton {
            icon.source: "image://theme/icon-l-add"

            Rectangle {
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    verticalCenter: parent.verticalCenter
                }
                width: parent.width
                height: parent.height
                color: "transparent"
                border.width: 1
                border.color: Theme.secondaryHighlightColor
            }

            onClicked: addPage()
        }
    }

    Loader {
        id: loader

        width: parent.width * dragDelegate.factor
        height: parent.height * dragDelegate.factor
        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Component.onCompleted: {
            if (!isAddButton) {
                loader.setSource("qrc:///qml/sfos/PageView.qml", {"page": page})
            } else {
                loader.sourceComponent = addButtonView;
            }
        }
    }
}
