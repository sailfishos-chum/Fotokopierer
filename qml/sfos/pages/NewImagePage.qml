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
import QtMultimedia 5.6
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import Fotokopierer 1.0

Page {
    id: page

    property alias acceptDestination: colpage.acceptDestination
    property alias acceptDestinationInstance: colpage.acceptDestinationInstance
    property alias acceptDestinationAction: colpage.acceptDestinationAction
    property alias acceptDestinationReplaceTarget: colpage.acceptDestinationReplaceTarget

    property ScanImage scanImage

    signal addPage()

    onStatusChanged: {
        if (status == PageStatus.Activating) {
            // Remove possibly old image
            scanImage.clear()
        }
    }

    onPageContainerChanged: {
        if (pageContainer == null) {
            console.log("NewImagePage closed")
            scanImage.clear()
        }
    }

    function processImage(imagePath, deleteOnCancel) {
        scanImage.loadFile(imagePath)
        scanImage.deleteOriginalOnClear = deleteOnCancel
        pageStack.push(Qt.resolvedUrl("CutPage.qml"), {"image": scanImage})
        pageStack.pushAttached(colpage)
    }

    function focusColor() {
        if (camera.lockStatus == Camera.Unlocked) {
            return Theme.highlightColor;
        } else if (camera.lockStatus == Camera.Searching) {
            return Theme.secondaryColor;
        } else {
            return Theme.primaryColor;
        }
    }

    Component {
        id: picker
        ImagePickerPage {
            id: picker

            // Note that this property might become unsupported in future
            popOnSelection: false

            onSelectedContentPropertiesChanged: {
                processImage(selectedContentProperties.filePath, false)
            }
        }
    }

    ColorizePage {
        id: colpage

        scanImage: page.scanImage

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

                camera.unlock()
                camera.focus.focusMode = Camera.FocusContinuous
                camera.focus.focusPointMode = Camera.FocusPointAuto

                focusCircle.x = viewArea.width / 2
                focusCircle.y = viewArea.height / 2

                processImage(path, true)
            }
        }

        flash.mode: Camera.FlashOff

        imageProcessing {
            sharpeningLevel: 1
        }

        exposure {
            exposureCompensation: -1.0
            exposureMode: Camera.ExposureAuto
        }

        focus {
            focusMode: Camera.FocusContinuous
            focusPointMode: Camera.FocusPointAuto
        }

        metaData.orientation: orientation
    }

    Rectangle {
        id: viewArea

        anchors.top: header.bottom
        anchors.bottom: buttons.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * Theme.iconSizeSmall

        VideoOutput {
            anchors.fill: parent

            fillMode: VideoOutput.Stretch
            orientation: camera.orientation
            focus: visible
            source: camera
        }

        Rectangle {
            id: focusCircle
            height: Theme.itemSizeHuge
            width: height
            radius: width / 2
            border.width: 2
            border.color: focusColor()
            color: "transparent"
            x: parent.width / 2
            y: parent.height / 2
            transform: Translate {
                x: -focusCircle.width / 2
                y: -focusCircle.height / 2
            }
        }

        MouseArea {
            anchors.fill: parent
            onClicked: {
                focusCircle.x = mouse.x
                focusCircle.y = mouse.y
                camera.focus.focusMode = Camera.FocusAuto
                camera.focus.focusPointMode = Camera.FocusPointCustom;
                camera.focus.setCustomFocusPoint(Qt.point((mouse.x / parent.width), (mouse.y / parent.height)));
                camera.searchAndLock()
            }
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
                    camera.imageCapture.captureToLocation(Fotokopierer.newImagePath())
                }
            }

            IconButton {
                width: parent.width / 3
                icon.source: "image://theme/icon-m-image"
                onClicked: pageStack.push(picker)
            }
        }
    }
}
