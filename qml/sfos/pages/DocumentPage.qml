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
import QtQml.Models 2.2
import Sailfish.Silica 1.0
import Fotokopierer 1.0

import ".."
import "../../common"

Page {
    id: docpage

    property var document
    property bool editing: false
    property bool dragging: false

    ScanImage {
        id: scanImage
    }

    Loader {
        id: newPage
    }

    onStatusChanged: {
        if (status == PageStatus.Active) {
            // ensure that the C++ memory of ScanImage is freed
            newPage.source = ""
            scanImage.clear()
        }
    }

    DelegateModel {
        id: visualModel
        delegate: PageDelegate {
            id: pageDelegate
            width: grid.cellWidth
            height: grid.cellHeight

            thumbnail: role_thumbnail != null && role_thumbnail != "" ? role_thumbnail : "image://theme/icon-l-image"
            pagenumber: DelegateModel.itemsIndex + 1
            creationTime: role_creationTime || new Date()

            isAddButton: role_thumbnail == null
            visible: !isAddButton || (!docpage.editing && !docpage.dragging)
            deleting: docpage.editing

            onPressed: {
                if (docpage.editing) {
                    docpage.editing = false
                    docpage.dragging = true
                    startDragging()
                }
            }

            // This timer is used to end dragging mode.
            //
            // The reason for using this timer is as follows. Dragging is
            // finished by an `onReleased` event. If the event handler would set
            // `dragging = false` the following `onClicked` event would trigger
            // an `addPage` or `openPage` command because it comes after the
            // `onReleased` event. However, because at this time `dragging ==
            // false` the page seems to be in "standard" non-editing mode
            // already. Therefore, instead of setting `dragging = false`
            // directly the event handler will start this time whose
            // `onTriggered` event will come after the `onClicked` event.
            Timer {
                id: endDraggingTimer
                interval: 0
                repeat: false
                onTriggered: docpage.dragging = false
            }

            onReleased: {
                if (docpage.dragging) {
                    endDraggingTimer.start()
                    endDragging()
                }
            }

            onClicked: {
                if (docpage.editing || docpage.dragging) {
                    docpage.editing = false
                } else if (isAddButton) {
                    addPage()
                } else {
                    openPage()
                }
            }

            onItemMoved: visualModel.model.move(from, to)

            function addPage() {
                newPage.source = Qt.resolvedUrl("NewImagePage.qml")
                newPage.item.scanImage = scanImage
                newPage.item.destination = docpage
                newPage.item.addPage.connect(function() {
                    document.addScannedPage(scanImage)
                })
                pageStack.push(newPage.item)
            }

            function openPage() {
                pageStack.push(Qt.resolvedUrl("PagePage.qml"), {
                    "image": role_result,
                    "title": qsTr("Page %1 of %2 (%3)").arg(pagenumber).arg(visualModel.count - 1).arg(creationTime.toLocaleString(Qt.locale(), Locale.ShortFormat)),
                })
            }

            onDeletePage: {
                docpage.dragging = false
                docpage.editing = false
                remorse.execute(pageDelegate, qsTr("Delete page"), function () {
                    document.deletePage(pageDelegate.DelegateModel.itemsIndex)
                })
            }

            RemorseItem {
                id: remorse
            }

            BusyIndicator {
                size: BusyIndicatorSize.Large
                anchors.centerIn: parent
                running: role_page != null && role_page.status != DocPage.Ready
            }
        }

        Component.onCompleted: {
            visualModel.model = document
            visualModel.items.insert({"role_thumbnail": null})
        }
    }

    SilicaGridView {
        id: grid

        anchors.fill: parent

        cellWidth: width / 2
        cellHeight: (height - Theme.itemSizeLarge) / 2

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 200 }
        }

        header: PageHeader {
            id: head
            title: document.title
        }

        model: visualModel

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                text: qsTr("Rename")
                onClicked: pageStack.push(Qt.resolvedUrl("RenamePage.qml"), { document: document })
            }
        }

        MouseArea {
            anchors.fill: grid

            propagateComposedEvents: true

            onClicked: {
                if (docpage.editing) {
                    var index = grid.indexAt(grid.contentX + mouse.x, grid.contentY + mouse.y)
                    if (index == -1 || index == visualModel.count - 1) {
                        docpage.editing = false
                    } else {
                        mouse.accepted = false
                    }
                } else {
                    mouse.accepted = false
                }
            }

            onPressed: {
                if (docpage.editing) {
                    var index = grid.indexAt(grid.contentX + mouse.x, grid.contentY + mouse.y)
                    if (index == -1 || index == visualModel.count - 1) {
                        docpage.editing = false
                    } else {
                        mouse.accepted = false
                    }
                }
            }

            onPressAndHold: {
                if (visualModel.count > 1) {
                    docpage.editing = true
                }
            }
        }

        property int scrollingDirection: 0

        // SmoothedAnimation {
        //     id: upAnimation
        //     target: grid
        //     property: "contentY"
        //     to: 0
        //     running: scrollingDirection == -1
        // }

        // SmoothedAnimation {
        //     id: downAnimation
        //     target: grid
        //     property: "contentY"
        //     to: grid.contentHeight - grid.height
        //     running: scrollingDirection == 1
        // }

        // scrollingDirection: {
        //     var yCoord = grid.mapFromItem(dragArea, 0, dragArea.mouseY).y;
        //     if (yCoord < scrollEdgeSize) {
        //             -1;
        //     } else if (yCoord > grid.height - scrollEdgeSize) {
        //         1;
        //     } else {
        //         0;
        //     }
        // }
    }
}
