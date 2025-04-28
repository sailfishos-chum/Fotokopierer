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
                    TestDocument.addPage(plain, colpage.image)
                    plain.destroy()
                })
            }
        }
    }

    DelegateModel {
        id: visualModel
        delegate: PageDelegate {
            width: grid.cellWidth
            height: grid.cellHeight
            page: role_page
            isAddButton: role_page == null

            onItemMoved: visualModel.model.move(from, to)
            onAddPage: pageStack.push(imagePickerPage)
        }

        Component.onCompleted: {
            visualModel.items.insert({"role_page": null})
        }
    }

    Component.onCompleted: {
        if (!TestDocument.load("/home/nemo/fotokopierer/doc1/doc.json", null)) {
            console.log("can't load document file")
        } else {
            visualModel.model = TestDocument
            visualModel.items.insert({"role_page": null})
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
            title: "Document"
        }

        model: visualModel

        VerticalScrollDecorator {}

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
