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
import Sailfish.Pickers 1.0
import Fotokopierer 1.0

import ".."
import "../../common"

Page {
    id: docpage

    property var document
    property bool editing: false
    property bool dragging: false

    Component {
        id: imagePickerPage
        ImagePickerPage {
            // Note that this property might become unsupported in future
            popOnSelection: false

            onSelectedContentPropertiesChanged: {
                var plain = Fotokopierer.loadPlainImage(selectedContentProperties.filePath)

                var CutPage = Qt.createComponent(Qt.resolvedUrl("CutPage.qml"))
                var cutpage = CutPage.createObject(docpage, {"source": plain})
                pageStack.push(cutpage)

                var ColorizePage = Qt.createComponent(Qt.resolvedUrl("ColorizePage.qml"))
                var colpage = ColorizePage.createObject(docpage,
                                                        {
                                                            "source": cutpage.image,
                                                            "acceptDestination": docpage,
                                                            "acceptDestinationAction": PageStackAction.Pop,
                                                        })
                pageContainer.pushAttached(colpage)
                colpage.accepted.connect(function() {
                    document.addPage(plain, colpage.image)
                })

                // Ensure the pages are destroyed once they are dropped from the
                // page stack. This is necessary so that the memory allocated by
                // the image classes is freed (the memory is allocated on the
                // C++ side and possibly not visible for the QML garbage
                // collector).
                cutpage.pageContainerChanged.connect(function() {
                    if (cutpage.pageContainer == null) {
                        colpage.destroy()
                        cutpage.destroy()
                        plain.destroy()
                    }
                })
            }
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
            zoom: docpage.editing

            onPressed: {
                if (docpage.editing) {
                    docpage.editing = false
                    docpage.dragging = true
                    startDragging()
                }
            }

            onReleased: {
                if (docpage.dragging) {
                    docpage.dragging = false
                    stopDragging()
                }
            }

            onClicked: {
                if (docpage.editing) {
                    docpage.editing = false
                } else if (isAddButton) {
                    addPage()
                } else {
                    openPage()
                }
            }

            onItemMoved: visualModel.model.move(from, to)

            function addPage() {
                pageStack.push(imagePickerPage)
            }

            function openPage() {
                console.log("open page")
            }

            IconButton {
                visible: docpage.editing && !isAddButton
                anchors { top: parent.top; right: parent.right }
                icon.source: "image://theme/icon-l-clear"
                onClicked: {
                    docpage.dragging = false
                    docpage.editing = false
                    remorse.execute(pageDelegate, qsTr("Delete page"), function () {
                        document.deletePage(pageDelegate.DelegateModel.itemsIndex)
                    })
                }
            }

            RemorseItem {
                id: remorse
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
