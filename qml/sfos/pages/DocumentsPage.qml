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

    property bool editing: false
    property bool deleting: false

    DelegateModel {
        id: visualModel
        model: DocumentList
        delegate: DocumentDelegate {
            id: docDelegate
            width: grid.cellWidth
            height: grid.cellHeight
            zoom: docpage.editing

            title: role_title
            pagecount: role_numPages
            creationTime: role_creationTime
            thumbnails: role_thumbnails

            isAddButton: role_thumbnails == null
            visible: !isAddButton || !docpage.editing

            enabled: true

            onClicked: {
                if (docpage.editing) {
                    docpage.editing = false
                } else if (isAddButton) {
                    addDocument()
                } else {
                    openDocument()
                }
            }

            function addDocument() {
                console.log("add document")
            }

            function openDocument() {
                pageStack.push(Qt.resolvedUrl("DocumentPage.qml"), {"document": role_document})
            }

            IconButton {
                visible: docpage.editing && !isAddButton
                anchors { top: parent.top; right: parent.right }
                icon.source: "image://theme/icon-l-clear"
                onClicked: {
                    docpage.editing = false
                    docpage.deleting = true
                    remorse.execute(docDelegate, qsTr("Delete document"), function () {
                        console.log("delete document")
                        //document.deletePage(docDelegate.DelegateModel.itemsIndex)
                    })
                }
            }

            RemorseItem {
                id: remorse
                onCanceled: docpage.deleting = false
                onTriggered: docpage.deleting = false
            }
        }

        Component.onCompleted: {
            visualModel.items.insert({
                "role_thumbnails": null,
                "role_numPages": 0,
                "role_title": "",
                "role_creationTime": "",
            })
        }
    }

    SilicaGridView {
        id: grid

        anchors.fill: parent

        cellWidth: width / 2
        cellHeight: (height - Theme.itemSizeLarge) / 2

        header: PageHeader {
            id: head
            title: qsTr("Documents")
        }

        model: visualModel

        VerticalScrollDecorator {}

        MouseArea {
            anchors.fill: grid

            propagateComposedEvents: true

            onPressAndHold: {
                if (visualModel.count > 1) {
                    docpage.editing = true
                }
            }
        }
    }
}
