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

    signal updateTouchPoints()

    ColorizeChooser {
        id: colorizer
        anchors.fill: parent
    }

    Repeater {
        model: 6
        CornerMarker {
            radius: 10
            minX: 0
            maxX: root.width
            minY: 0
            maxY: root.height

            onDragged: colorizer.setColorAngle(index, pos2angle(markerPos))

            Connections {
                target: colorizer
                onColorAnglesChanged: updateAngle()
            }

            Connections {
                target: root
                onUpdateTouchPoints: updateAngle()
            }

            function updateAngle() {
                var angle = colorizer.colorAngle(index) / 180 * Math.PI
                setCenter(Qt.point(Math.cos(angle) * _radius + root.width / 2,
                                   -Math.sin(angle) * _radius + root.height / 2))
            }
        }
    }

    function chooser () {
        return colorizer
    }

    function pos2angle(pos) {
        var dx = pos.x - root.width / 2
        var dy = -(pos.y - root.height / 2)
        if (dx == 0) {
            if (dy > 0) {
                return 90;
            } else {
                return 270;
            }
        } else {
            var angle = Math.atan(dy / dx) / Math.PI * 180
            if (dx < 0) {
                return angle + 180;
            } else if (angle < 0) {
                return angle + 360;
            } else {
                return angle;
            }
        }
    }

    on_RadiusChanged: updateTouchPoints()
}
