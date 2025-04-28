/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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
import QtQuick 2.0

import "../common"

DragDelegate {
    id: dragDelegate

    property string thumbnail
    property int pagenumber
    property date creationTime

    property bool isAddButton: false

    property alias deleting: deletable.deleting
    property bool marked: false

    signal deletePage()

    canBeDragged: !isAddButton

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
        }
    }

    Component {
        id: pageView
        Item {
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: Theme.highlightColor
            }

            Rectangle {
                color: Theme.secondaryHighlightColor

                anchors.top: parent.top
                anchors.bottom: info.top
                anchors.left: parent.left
                anchors.right: parent.right

                Image {
                    source: dragDelegate.thumbnail
                    cache: false
                    anchors.fill: parent
                    fillMode: Image.PreserveAspectFit
                }
            }

            Rectangle {
                anchors.fill: parent
                color: Theme.secondaryHighlightColor
                opacity: 0.9
                z: 1
                visible: dragDelegate.marked
            }

            Column {
                id: info

                anchors.left: parent.left
                anchors.right: parent.right
                anchors.bottom: parent.bottom

                Text {
                    width: parent.width
                    color: Theme.highlightColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: qsTr("Page %1").arg(dragDelegate.pagenumber)
                }

                Text {
                    width: parent.width
                    color: Theme.highlightColor
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter
                    elide: Text.ElideRight
                    text: dragDelegate.creationTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
                }
            }
        }
    }

    DeletableItem {
        id: deletable

        anchors.fill: parent

        onDeleteItem: deletePage()

        Loader {
            id: loader

            anchors {
                fill: parent
                leftMargin: 0.05 * parent.width
                rightMargin: 0.05 * parent.width
                topMargin: 0.05 * parent.height
                bottomMargin: 0.05 * parent.height
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
}
