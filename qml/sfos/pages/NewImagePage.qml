/*
 * Copyright (c) 2018, 2020 Frank Fischer <frank-fischer@shadow-soft.de>
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

    allowedOrientations: Orientation.Portrait

    property var acceptDestination
    property var acceptDestinationAction
    property Page acceptDestinationInstance
    property Page acceptDestinationReplaceTarget

    property bool _haveResolution: false

    signal addPage()

    onStatusChanged: {
        if (status == PageStatus.Activating) {
            // Remove possibly old image
            Scanner.clear()
        }
        if (status == PageStatus.Active) {
            camera.cameraState = Camera.ActiveState
        }
    }

    onPageContainerChanged: {
        if (pageContainer == null) {
            console.log("NewImagePage closed")
            Scanner.clear()
        }
    }

    function processImage(imagePath, deleteOnCancel) {
        Scanner.loadFile(imagePath)
        Scanner.deleteOriginalOnClear = deleteOnCancel
        pageStack.push(cutpage)
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

    Component {
        id: cutpage
        CutPage {}
    }

    Component {
        id: colpage
        ColorizePage {
            onAccepted: addPage()

            acceptDestination: page.acceptDestination
            acceptDestinationAction: page.acceptDestinationAction
            acceptDestinationReplaceTarget: page.acceptDestinationReplaceTarget

            onAcceptDestinationInstanceChanged: {
                page.acceptDestinationInstance = acceptDestinationInstance
            }
        }
    }

    PageHeader {
        id: header
        title: qsTr("New Picture")
    }

    Camera {
        id: camera

        cameraState: Camera.UnloadedState

        viewfinder {
            resolution: Qt.size(640, 480)
        }

        imageCapture {
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

        onCameraStatusChanged: {
            if (cameraStatus == Camera.ActiveStatus && !_haveResolution) {
                var res = Fotokopierer.defaultResolution(imageCapture, viewArea.width, viewArea.height)
                if (res.width > 0) {
                    console.log("set resolution: " + res)
                    imageCapture.setResolution(res)
                }
                _haveResolution = true
            }
        }
    }

    Item {
        id: viewArea

        anchors.top: header.bottom
        anchors.bottom: buttons.top
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - 2 * Theme.iconSizeSmall

        VideoOutput {
            anchors.fill: parent

            visible: camera.cameraStatus == Camera.ActiveStatus && _haveResolution
            fillMode: VideoOutput.Stretch
            orientation: camera.orientation
            focus: visible
            source: camera
        }

        Rectangle {
            id: focusCircle
            visible: camera.cameraStatus == Camera.ActiveStatus
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

        BusyIndicator {
            size: BusyIndicatorSize.Large
            anchors.centerIn: parent
            running: camera.cameraStatus != Camera.ActiveStatus
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
