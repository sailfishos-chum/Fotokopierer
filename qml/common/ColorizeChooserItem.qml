/*
 * Copyright (c) 2021 Frank Fischer <frank-fischer@shadow-soft.de>
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
import Fotokopierer 1.0

Item {
    id: root

    property alias blackLevel: colorizer.blackLevel

    property real _radius: Math.min(width, height) / 2

    ColorizeChooser {
        id: colorizer
        anchors.fill: parent

        onColorAnglesChanged: updateTouchPoints()
    }

    CornerMarker {
        id: marker0
        radius: 10
        minX: 0
        maxX: root.width
        minY: 0
        maxY: root.height
    }

    Component.onCompleted: updateTouchPoints()

    on_RadiusChanged: updateTouchPoints()

    function chooser () {
        return colorizer
    }

    function updateTouchPoints() {
        marker0.setCenter(Qt.point(Math.cos(colorizer.colorAngle(0) / 180 * Math.PI) * _radius + root.width / 2,
                                   -Math.sin(colorizer.colorAngle(0) / 180 * Math.PI) * _radius + root.height / 2))
    }
}
