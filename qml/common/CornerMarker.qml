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

Item {
    id: root

    Drag.active: mouseArea.drag.active
    Drag.hotSpot.x: width / 2
    Drag.hotSpot.y: height / 2

    property point center: Qt.point(x + radius, y + radius)
    property real radius: 10
    property color color: "white"
    property real fillOpacity: 0.5
    property real linewidth: 1

    property real minX
    property real maxX
    property real minY
    property real maxY

    property bool dragActive: false

    x: centerx - radius
    y: centery - radius
    width: radius * 2
    height: radius * 2

    Rectangle {
        id: rectangle

        anchors.fill: parent
        antialiasing: true
        radius: width / 2
        color: Qt.rgba(parent.color.r, parent.color.g, parent.color.b, parent.fillOpacity)
        border.color: parent.color
        border.width: parent.linewidth
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        drag.target: parent

        drag.minimumX: root.minX
        drag.maximumX: root.maxX
        drag.minimumY: root.minY
        drag.maximumY: root.maxY

        onPressed: dragActive = true
        onReleased: dragActive = false
    }
}
