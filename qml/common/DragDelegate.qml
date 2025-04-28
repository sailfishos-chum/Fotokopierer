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

MouseArea {
    id: dragArea

    property bool dragEnabled: true
    property bool held: false
    property int sourceIndex: 0

    default property alias data: content.data

    signal itemMoved(int from, int to)

    drag.target: held ? content : undefined
    drag.axis: Drag.XAndYAxis

    onPressAndHold: {
        held = true
        sourceIndex = DelegateModel.itemsIndex
    }
    onReleased: {
        held = false
        itemMoved(sourceIndex, DelegateModel.itemsIndex)
    }

    states: State {
        when: dragArea.held
        ParentChange { target: content; parent: dragArea.parent }
        AnchorChanges {
            target: content
            anchors {
                horizontalCenter: undefined
                verticalCenter: undefined
            }
        }
    }

    DropArea {
        anchors { fill: parent; margins: 10 }
        onEntered: {
            dragArea.DelegateModel.model.items.move(drag.source.DelegateModel.itemsIndex, dragArea.DelegateModel.itemsIndex)
        }
        enabled: dragArea.dragEnabled
    }

    Item {
        id: content

        opacity: dragArea.held ? 0.5 : 1.0

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }
        width: dragArea.width
        height: dragArea.height

        Drag.active: dragArea.held
        Drag.source: dragArea
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2
    }
}
