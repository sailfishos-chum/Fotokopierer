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

import QtQuick 2.0
import QtQuick.Layouts 1.0
import Sailfish.Silica 1.0
import Fotokopierer 1.0

Page {
    id: thepage

    property string title
    property var page


    Flickable {
        id: flick

        anchors.fill: parent
        contentWidth: imageView.width;
        contentHeight: imageView.height
        clip: true

        onHeightChanged: if (imageView.status === Image.Ready) imageView.fitToScreen();

        PageHeader {
            title: thepage.title
        }

        Item {
            id: imageView

            width: Math.max(img.width * img.scale, flick.width)
            height: Math.max(img.height * img.scale, flick.height)

            Image {
                id: img

                property real prevScale

                width: flick.width
                height: flick.height

                function fitToScreen() {
                    scale = Math.min(flick.width / width, flick.height / height, 1)
                    pinchArea.minScale = scale
                    prevScale = scale
                }

                anchors.centerIn: parent
                fillMode: Image.PreserveAspectFit
                cache: false
                asynchronous: true
                source: page.resultUrl
                smooth: !flick.moving

                onStatusChanged: {
                    if (status == Image.Ready) {
                        fitToScreen()
                        loadedAnimation.start()
                    }
                }

                NumberAnimation {
                    id: loadedAnimation
                    target: img
                    property: "opacity"
                    duration: 250
                    from: 0; to: 1
                    easing.type: Easing.InOutQuad
                }

                onScaleChanged: {
                    if ((width * scale) > flick.width) {
                        var xoff = (flick.width / 2 + flick.contentX) * scale / prevScale;
                        flick.contentX = xoff - flick.width / 2
                    }
                    if ((height * scale) > flick.height) {
                        var yoff = (flick.height / 2 + flick.contentY) * scale / prevScale;
                        flick.contentY = yoff - flick.height / 2
                    }
                    prevScale = scale
                }

                BusyIndicator {
                    size: BusyIndicatorSize.Large
                    anchors.centerIn: parent
                    running: img.status != Image.Ready
                }
            }
        }

        PinchArea {
            id: pinchArea

            property real minScale: 1.0
            property real maxScale: 3.0

            anchors.fill: parent
            enabled: img.status === Image.Ready
            pinch.target: img
            pinch.minimumScale: minScale * 0.5 // This is to create "bounce back effect"
            pinch.maximumScale: maxScale * 1.5 // when over zoomed

            onPinchFinished: {
                flick.returnToBounds()
                if (img.scale < pinchArea.minScale) {
                    bounceBackAnimation.to = pinchArea.minScale
                    bounceBackAnimation.start()
                }
                else if (img.scale > pinchArea.maxScale) {
                    bounceBackAnimation.to = pinchArea.maxScale
                    bounceBackAnimation.start()
                }
            }

            NumberAnimation {
                id: bounceBackAnimation
                target: img
                duration: 250
                property: "scale"
                from: img.scale
            }
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
                icon.source: "image://theme/icon-m-edit"
                Layout.fillWidth: true
                onClicked: _editImage()
            }
        }
    }

    Component {
        id: cutpage
        CutPage {}
    }

    Component {
        id: colpage
        ColorizePage {
            onAccepted: {
                Scanner.updatePage(page)
            }

            acceptDestination: thepage
            acceptDestinationAction: PageStackAction.Pop
        }
    }

    function _editImage() {
        Scanner.loadPage(page)
        Scanner.deleteOriginalOnClear = false
        pageStack.push(cutpage)
        pageStack.pushAttached(colpage)
    }
}
