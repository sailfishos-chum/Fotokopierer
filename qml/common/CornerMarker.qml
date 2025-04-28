/*
 * Copyright (c) 2018, 2019, 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

/// CornerMarker is an dragable marker. It should cover
/// the whole allowed area.
Item {
    id: root

    property real radius: 10
    property color color: "white"
    property real fillOpacity: 0.5
    property real linewidth: 1

    property real minX: 0
    property real maxX: width
    property real minY: 0
    property real maxY: height

    property point center: Qt.point(0, 0)

    /// Whether the point is currently dragged
    property bool dragActive: false

    /// Raised if this drag point is being dragged to another position
    signal dragged(point position)

    anchors.fill: parent

    Item {
        id: dragArea

        width: root.radius * 2
        height: root.radius * 2
        x: root.width / 2 - root.radius
        y: root.height / 2 - root.radius

        Drag.active: mouseArea.drag.active
        Drag.hotSpot.x: width / 2
        Drag.hotSpot.y: height / 2

        MouseArea {
            id: mouseArea
            anchors.fill: parent
            drag.target: parent

            drag.minimumX: root.minX - root.radius
            drag.maximumX: root.maxX - root.radius
            drag.minimumY: root.minY - root.radius
            drag.maximumY: root.maxY - root.radius

            onPressed: root.dragActive = true
            onReleased: {
                root.dragActive = false
                fixDragArea()
            }
        }

        onXChanged: {
            if (root.dragActive) {
                root.dragged(Qt.point(dragArea.x + root.radius, dragArea.y + root.radius))
            }
        }

        onYChanged: {
            if (root.dragActive) {
                root.dragged(Qt.point(dragArea.x + root.radius, dragArea.y + root.radius))
            }
        }
    }

    Rectangle {
        id: marker

        width: root.radius * 2
        height: root.radius * 2
        x: center.x - root.radius
        y: center.y - root.radius

        antialiasing: true
        radius: width / 2
        color: Qt.rgba(root.color.r, root.color.g, root.color.b, root.fillOpacity)
        border.color: root.color
        border.width: root.linewidth
    }

    onCenterChanged: {
        if (!dragActive) {
            fixDragArea()
        }
    }

    onRadiusChanged: fixDragArea()

    Component.onCompleted: fixDragArea()

    function fixDragArea() {
        dragArea.x = center.x - root.radius
        dragArea.y = center.y - root.radius
    }
}
