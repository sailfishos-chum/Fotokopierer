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
import QtGraphicalEffects 1.0
import Fotokopierer 1.0

Item {
    id: zoom

    property real scaleFactor : 5
    property real imagex
    property real imagey
    property real size : 100
    property color color: "white"
    property color crossColor: "green"

    property RotImage image

    width: size
    height: size

    Rectangle {
        id: frame
        radius: size / 2
        anchors.fill: parent
        color: "black"
        border.width: size / 20
        border.color: zoom.color
    }

    DropShadow {
        anchors.fill: parent
        horizontalOffset: 3
        verticalOffset: 3
        radius: 8
        samples: 17
        color: "#80000000"
        source: frame
    }

    Item {
        id: zoombox

        layer.enabled: true
        layer.effect: OpacityMask {
            maskSource: Item {
                width: zoombox.width
                height: zoombox.height
                Rectangle {
                    anchors.fill: parent
                    radius: width / 2
                }
            }
        }

        PlainImage {
            id: zoomimg
            width: image.paintedWidth * zoom.scaleFactor
            height: image.paintedHeight * zoom.scaleFactor
            source: image
            x: (-zoom.imagex + (pane.width - image.paintedWidth) / 2) * zoom.scaleFactor - zoombox.anchors.leftMargin
            y: (-zoom.imagey + (pane.height - image.paintedHeight) / 2) * zoom.scaleFactor - zoombox.anchors.rightMargin
        }

        anchors.fill: parent
        anchors.margins: frame.border.width
    }

    Canvas {
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.clearRect(0, 0, width, height)
            ctx.strokeStyle = zoom.crossColor
            ctx.beginPath()
            ctx.moveTo(x + width / 2 - width / 5, y + height / 2)
            ctx.lineTo(x + width / 2 + width / 5, y + height / 2)
            ctx.moveTo(x + width / 2, y + height / 2 - height / 5)
            ctx.lineTo(x + width / 2, y + height / 2 + height / 5)
            ctx.stroke()
        }
    }
}
