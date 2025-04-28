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

import QtQuick 2.0
import QtQuick.Layouts 1.0
import QtMultimedia 5.6
import Sailfish.Silica 1.0
import Fotokopierer 1.0

Page {
    id: page

    property Page destination

    signal addPage()

    CutPage { id: cutpage }

    ColorizePage {
        id: colpage

        acceptDestination: destination
        acceptDestinationAction: PageStackAction.Pop

        onAccepted: addPage()
    }

    PageHeader {
        id: header
        title: qsTr("New Picture")
    }

    Camera {
        id: camera

        viewfinder {
            resolution: Qt.size(640, 480)
        }

        imageCapture {
            resolution: Qt.size(4000, 3000)
            onImageCaptured: {
                //photoPreview.source = preview
                console.log("image captured: " + preview)
            }
            onImageSaved: {
                console.log("save image: " + path)
                Scanner.loadFile(path)
                pageStack.push(cutpage)
                pageStack.pushAttached(colpage)
            }
        }

        focus {
            focusMode: Camera.FocusContinuous
            focusPointMode: Camera.FocusPointCenter
        }

        flash.mode: Camera.FlashOff

        imageProcessing {
            sharpeningLevel: 1
        }

        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposurePortrait
        }
    }

    Rectangle {
        anchors.top: header.bottom
        anchors.bottom: buttons.top
        anchors.left: parent.left
        anchors.right: parent.right

        VideoOutput {
            anchors.fill: parent

            fillMode: VideoOutput.Stretch

            focus: visible
            source: camera
        }
    }

    DockedPanel {
        id: buttons
        open: true

        width: parent.width
        height: Theme.iconSizeLarge
        dock: Dock.Bottom

        Row {
            anchors.fill: parent

            IconButton {
                width: parent.width / 3
                icon.source:
                camera.flash.mode == Camera.FlashOff ?
                    "image://theme/icon-camera-flash-off" :
                    camera.flash.mode == Camera.FlashAuto ?
                    "image://theme/icon-camera-flash-automatic" :
                    "image://theme/icon-camera-flash-on"
                onClicked: {
                    if (camera.flash.mode == Camera.FlashOff) {
                        camera.flash.mode = Camera.FlashOn
                    } else if (camera.flash.mode == Camera.FlashOn) {
                        camera.flash.mode = Camera.FlashAuto
                    } else {
                        camera.flash.mode = Camera.FlashOff
                    }
                }
            }

            IconButton {
                width: parent.width / 3
                icon.source: "image://theme/icon-camera-shutter-release"
                onClicked: {
                    camera.imageCapture.capture()
                }
            }
        }
    }
}
