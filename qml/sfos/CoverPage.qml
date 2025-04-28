/*
 * Copyright (c) 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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

import QtQuick 2.0
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0
import Fotokopierer 1.0

CoverBackground {
    property var document // document to show, usually the document opened last
    property var _thumbnails: document ? document.thumbnails : []
    property var _numPages: document ? document.numPages : 0

    signal newPicture()

    Label {
        id: nodoc

        visible: !document

        anchors.top: parent.top
        anchors.bottom: coverActionArea.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width * 0.9
        height: parent.height * 0.9

        font.pixelSize: Theme.fontSizeHuge
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        wrapMode: Text.WordWrap

        text: qsTr("Take a new picture")
    }

    Label {
        id: pages

        visible: document

        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        font.pixelSize: Theme.fontSizeMedium

        text: qsTr("Pages: %1").arg(_numPages)
    }

    Item {
        visible: document

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: pages.bottom
        anchors.bottom: coverActionArea.top

        Item {
            width: parent.width * 0.9
            height: parent.height * 0.9
            anchors.centerIn: parent

            Repeater {
                id: repeater
                model: _thumbnails

                delegate: Image {
                    source: _thumbnails[_thumbnails.length - index - 1]

                    anchors.fill: parent
                    anchors.leftMargin: (_thumbnails.length - index - 1) * 0.05 * parent.width
                    anchors.rightMargin: index * 0.05 * parent.width
                    anchors.topMargin: (_thumbnails.length - index - 1) * 0.05 * parent.height
                    anchors.bottomMargin: index * 0.05 * parent.height

                    fillMode: Image.PreserveAspectFit

                    ColorOverlay {
                        anchors.fill: parent
                        source: parent
                        color: "black"
                        opacity: (_thumbnails.length - index - 1) * 0.1
                    }
                }
            }
        }
    }

    CoverActionList {
        CoverAction {
            iconSource: "image://theme/icon-m-camera"
            onTriggered: newPicture()
        }
    }
}
