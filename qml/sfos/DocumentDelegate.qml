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
import QtGraphicalEffects 1.0
import QtQuick 2.0

import "../common"

MouseArea {
    id: dragDelegate

    property string title
    property int pagecount
    property date creationTime
    property var thumbnails

    property bool isAddButton: false

    // This property is true if the Item can be deleted.
    // It will be shrunk and a delete button will be shown.
    property alias deleting: deletable.deleting

    signal deleteDocument()

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
        id: docView
        Item {
            Rectangle {
                anchors.fill: parent
                color: "transparent"
                border.color: Theme.highlightColor
            }

            Text {
                id: titleText
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right

                color: Theme.highlightColor
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter

                elide: Text.ElideRight

                text: dragDelegate.title
            }

            Rectangle {
                color: Theme.secondaryHighlightColor

                anchors.top: titleText.bottom
                anchors.bottom: info.top
                anchors.left: parent.left
                anchors.right: parent.right

                Repeater {
                    model: dragDelegate.thumbnails

                    delegate: Image {
                        source: dragDelegate.thumbnails[thumbnails.length - index - 1]
                        cache: false

                        anchors.fill: parent
                        anchors.leftMargin: (thumbnails.length - index - 1) * 0.05 * parent.width
                        anchors.rightMargin: index * 0.05 * parent.width
                        anchors.topMargin: (thumbnails.length - index - 1) * 0.05 * parent.height
                        anchors.bottomMargin: index * 0.05 * parent.height

                        fillMode: Image.PreserveAspectFit

                        ColorOverlay {
                            anchors.fill: parent
                            source: parent
                            color: "black"
                            opacity: (thumbnails.length - index - 1) * 0.1
                        }
                    }
                }
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
                    text: qsTr("Pages: %1").arg(dragDelegate.pagecount)
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
        onDeleteItem: deleteDocument()

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
                    loader.sourceComponent = docView;
                } else {
                    loader.sourceComponent = addButtonView;
                }
            }
        }
    }
}
