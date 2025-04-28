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
import QtQml.Models 2.2
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0
import Fotokopierer 1.0

import ".."
import "../../common"

Page {
    id: docpage

    property var document: null // the document, may be null

    // Allowed states are "Normal", "Editing", "Dragging", "Marking"
    property string state: "Normal"

    // The number of marked pages in marking mode.
    property int _nmarked: 0

    Loader {
        id: newPage
    }

    onStatusChanged: {
        if (status == PageStatus.Active) {
            // ensure that the C++ memory of Scanner is freed
            newPage.source = ""
            Scanner.clear()
        }
    }

    on_NmarkedChanged: {
        if (_nmarked == 0) {
            // The number of marked pages dropped to 0 -> stop marking state
            state = "Normal"
        }
    }

    DelegateModel {
        id: visualModel
        delegate: PageDelegate {
            id: pageDelegate
            width: grid.cellWidth
            height: grid.cellHeight

            thumbnail: (role_thumbnail && role_thumbnail != "") ? role_thumbnail : "image://theme/icon-l-image"
            pagenumber: DelegateModel.itemsIndex + 1
            creationTime: role_creationTime || new Date()

            isAddButton: role_thumbnail ? false : true
            visible: !isAddButton || (docpage.state == "Normal")
            deleting: docpage.state == "Editing"
            marked: role_selected

            onPressed: {
                if (docpage.state == "Editing") {
                    docpage.state = "Dragging"
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
                onTriggered: docpage.state = "Normal"
            }

            onReleased: {
                if (docpage.state == "Dragging") {
                    endDraggingTimer.start()
                    endDragging()
                }
            }

            onClicked: {
                if (docpage.state == "Editing" || docpage.state == "Dragging") {
                    docpage.state = "Normal"
                } else if (docpage.state == "Marking") {
                    if (role_selected) {
                        role_selected = false
                        docpage._nmarked -= 1
                    } else {
                        role_selected = true
                        docpage._nmarked += 1
                    }
                } else if (isAddButton) {
                    addPage()
                } else {
                    var title = qsTr("Page %1 of %2 (%3)").arg(pagenumber).arg(visualModel.count - 1).arg(creationTime.toLocaleString(Qt.locale(), Locale.ShortFormat))
                    openPage(role_page, title)
                }
            }

            onItemMoved: visualModel.model.move(from, to)

            onDeletePage: {
                docpage.state = "Normal"
                remorse.execute(pageDelegate, qsTr("Delete page"), function () {
                    if (document) {
                        document.deletePage(pageDelegate.DelegateModel.itemsIndex)
                    }
                })
            }

            RemorseItem {
                id: remorse
            }

            BusyIndicator {
                size: BusyIndicatorSize.Large
                anchors.centerIn: parent
                running: (role_page ? true : false) && role_page.status != DocPage.Ready
            }
        }
    }

    SilicaGridView {
        id: grid

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: parent.top
        anchors.bottom: buttons.top

        cellWidth: width / 2
        cellHeight: (height - Theme.itemSizeLarge) / 2

        displaced: Transition {
            NumberAnimation { properties: "x,y"; duration: 200 }
        }

        header: PageHeader {
            id: head
            title: document ? document.title : ""
        }

        model: visualModel

        VerticalScrollDecorator {}

        PullDownMenu {
            MenuItem {
                text: PageClipboard.empty ?
                      qsTr("Paste pages") : qsTr("Paste pages (%1)").arg(PageClipboard.numPages)
                onClicked: document.pastePages()
                enabled: !PageClipboard.empty
            }

            MenuItem {
                text: qsTr("Select pages")
                onClicked: docpage.state = "Marking"
                enabled: (document ? true : false) && docpage.state == "Normal"
            }

            MenuItem {
                text: qsTr("Export to pdf")
                onClicked: document.exportToPdf()
                enabled: document ? true : false
            }

            MenuItem {
                text: qsTr("Rename")
                onClicked: pageStack.push(Qt.resolvedUrl("RenamePage.qml"), { document: document })
                enabled: document ? true : false
            }
        }

        MouseArea {
            anchors.fill: grid

            propagateComposedEvents: true

            onClicked: {
                if (docpage.state == "Editing") {
                    var index = grid.indexAt(grid.contentX + mouse.x, grid.contentY + mouse.y)
                    if (index == -1 || index == visualModel.count - 1) {
                        docpage.state = "Normal"
                    } else {
                        mouse.accepted = false
                    }
                } else {
                    mouse.accepted = false
                }
            }

            onPressed: {
                if (docpage.state == "Editing") {
                    var index = grid.indexAt(grid.contentX + mouse.x, grid.contentY + mouse.y)
                    if (index == -1 || index == visualModel.count - 1) {
                        docpage.state = "Normal"
                    } else {
                        mouse.accepted = false
                    }
                }
            }

            onPressAndHold: {
                if (visualModel.count > 1) {
                    docpage.state = "Editing"
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

    DockedPanel {
        id: buttons

        width: parent.width
        height: Theme.iconSizeLarge
        dock: Dock.Bottom
        open: docpage.state == "Marking"

        RowLayout {
            id: buttonRow
            anchors { left: parent.left; right: parent.right }
            IconButton {
                id: copybutton
                height: buttons.height
                Layout.fillWidth: true
                icon.source: Qt.resolvedUrl("/icons/toolbar-copy.svg")
                icon.fillMode: Image.PreserveAspectFit
                icon.height: Theme.iconSizeMedium
                onClicked: {
                    document.copySelectedPages()
                    document.clearSelection()
                    buttons.open = false
                    docpage._nmarked = 0
                }

                ColorOverlay {
                    anchors.fill: parent
                    source: parent
                    color: Theme.primaryColor
                }
            }

            IconButton {
                id: cutbutton
                height: buttons.height
                Layout.fillWidth: true
                icon.source: Qt.resolvedUrl("/icons/toolbar-cut.svg")
                icon.height: Theme.iconSizeMedium
                icon.fillMode: Image.PreserveAspectFit
                onClicked: {
                    document.cutSelectedPages()
                    document.clearSelection()
                    buttons.open = false
                    docpage._nmarked = 0
                }

                ColorOverlay {
                    anchors.fill: parent
                    source: parent
                    color: Theme.primaryColor
                }
            }

            IconButton {
                height: buttons.height
                Layout.fillWidth: true
                icon.source: "image://theme/icon-m-delete"
                icon.height: Theme.iconSizeMedium
                icon.fillMode: Image.PreserveAspectFit
                onClicked: console.log("Delete")

            }
            IconButton {
                height: buttons.height
                Layout.fillWidth: true
                icon.source: "image://theme/icon-m-close"
                icon.height: Theme.iconSizeMedium
                icon.fillMode: Image.PreserveAspectFit
                onClicked: {
                    document.clearSelection()
                    buttons.open = false
                    docpage._nmarked = 0
                }
            }
        }
    }

    Component {
        id: overwritedlg

        Dialog {
            property string filename

            DialogHeader {
                id: header

                width: parent.width
                anchors.top: parent.top
            }

            Label {
                anchors.top: header.bottom
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right

                font.pixelSize: Theme.fontSizeHuge
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.WordWrap

                text: qsTr("Overwrite existing file <%1>?").arg(filename)
            }

            onAccepted: document.exportToPdf(true)
        }
    }

    // This is a dummy model only showing the "AddPage". It is used if document
    // == null, i.e. if no document is associated with this page.
    //
    // The only purpose is a nice peek right before accepting a new image (this
    // is the only situation where the page is shown without an associated
    // model).
    ListModel {
        id: emptymodel

        ListElement {
            role_thumbnail: false
            role_creationTime: false
            role_page: false
            role_selected: false
        }
    }

    // This timer is only used as a workaround for an unexpected crash.
    //
    // If the "AddPage" is added immediately when the document has been changed,
    // the program crashed. We use the timer to delay the addition of the
    // "AddPage", probably so that the DelegateModel is properly initialized.
    Timer {
        id: addpagetimer
        interval: 1
        running: false
        repeat: false
        onTriggered: visualModel.items.insert({"role_thumbnail": false, "role_selected": false})
    }

    onDocumentChanged: {
        // remove the "AddPage" (if exists)
        if (visualModel.items.count > 0) {
            visualModel.items.remove(0, visualModel.items.count)
        }
        if (document) {
            // a document has been specified, use as model and connect signals
            visualModel.model = document
            document.errorPdfExists.connect(_onPdfExists)
            document.exportToPdfFinished.connect(_onExportToPdfFinished)
            // add the "AddPage" (we use the timer as adding the page
            // immediately crashes the program)
            addpagetimer.running = true
        } else {
            // no document has been specified, use the dummy model
            visualModel.model = emptymodel
        }
    }

    Rectangle {
        anchors.fill: parent
        color: Theme.secondaryHighlightColor
        opacity: 0.5
        visible: (document && document.status == Document.Exporting) ? true : false

        BusyIndicator {
            size: BusyIndicatorSize.Large
            anchors.centerIn: parent
            running: parent.visible
        }
    }

    function addPage() {
        if (!document) {
            return
        }
        newPage.source = Qt.resolvedUrl("NewImagePage.qml")
        newPage.item.acceptDestination = docpage
        newPage.item.acceptDestinationAction = PageStackAction.Pop
        newPage.item.addPage.connect(function() {
            Scanner.addPage(document)
        })
        pageStack.push(newPage.item)
    }

    function openPage(page, title) {
        pageStack.push(Qt.resolvedUrl("PagePage.qml"), {
            "page": page,
            "title": title,
        })
    }

    function _onPdfExists(filename) {
        pageStack.push(overwritedlg, { filename: filename })
    }

    function _onExportToPdfFinished(path) {
        Qt.openUrlExternally(path)
    }
}
