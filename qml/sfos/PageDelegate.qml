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

    property string thumbnail
    property int pagenumber
    property date creationTime
    property bool isAddButton: false

    canBeDragged: !isAddButton

    signal addPage()

    onPressed: startDragging()
    onReleased: endDragging()

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

    Component {
        id: pageView
        PageView {
            thumbnail: dragDelegate.thumbnail
            pagenumber: dragDelegate.pagenumber
            creationTime: dragDelegate.creationTime
        }
    }

    Loader {
        id: loader

        width: parent.width * 0.9 * (dragDelegate.dragEnabled ? 0.9 : 1.0)
        height: parent.height * 0.9 * (dragDelegate.dragEnabled ? 0.9 : 1.0)

        Behavior on width {
            NumberAnimation { duration: 100 }
        }

        Behavior on height {
            NumberAnimation { duration: 100 }
        }

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Component.onCompleted: {
            if (!isAddButton) {
                loader.sourceComponent = pageView;
            } else {
                loader.sourceComponent = addButtonView;
            }
        }
    }
}
