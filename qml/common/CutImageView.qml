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
import Fotokopierer 1.0

Item {
    id: pane

    property ScanImage scanImage

    property real markerRadius: Math.min(width, height) / 25
    property color markerColor: "white"

    property color lineColor: "green"
    property color invalidLineColor: "red"

    property bool valid: true

    property point tl : mapPoint(topleft.center)
    property point tr : mapPoint(topright.center)
    property point br : mapPoint(bottomright.center)
    property point bl : mapPoint(bottomleft.center)

    FilterImage {
        id: image

        image: scanImage
        filterType: ScanImage.Rotate

        anchors.fill: parent
    }

    function rotateLeft() {
        console.log("rotateLeft")
        image.filter.orientation -= 1
    }

    function rotateRight() {
        console.log("rotateRight")
        image.filter.orientation += 1
    }

    function selectAll() {
        topleft.x = (pane.width - image.paintedWidth) / 2 - markerRadius;
        topleft.y = (pane.height - image.paintedHeight) / 2 - markerRadius;
        bottomright.x = (pane.width + image.paintedWidth) / 2 - markerRadius;
        bottomright.y = (pane.height + image.paintedHeight) / 2 - markerRadius;
        topright.x = bottomright.x
        topright.y = topleft.y
        bottomleft.x = topleft.x
        bottomleft.y = bottomright.y
    }

    function selectAuto() {
        var points = cutimage.autoDetectCutRect()
        var offx = (pane.width - image.paintedWidth) / 2 - markerRadius
        var offy = (pane.height - image.paintedHeight) / 2 - markerRadius
        var w = image.paintedWidth
        var h = image.paintedHeight
        topleft.x = points[0].x * w + offx
        topleft.y = points[0].y * h + offy
        topright.x = points[1].x * w + offx
        topright.y = points[1].y * h + offy
        bottomright.x = points[2].x * w + offx
        bottomright.y = points[2].y * h + offy
        bottomleft.x = points[3].x * w + offx
        bottomleft.y = points[3].y * h + offy
    }

    function cutImage() {
        cutimage.setCutBox(
            mapPoint(topleft.center),
            mapPoint(topright.center),
            mapPoint(bottomright.center),
            mapPoint(bottomleft.center))
    }

    Canvas {
        id: frame
        anchors.fill: parent
        onPaint: {
            var ctx = getContext("2d")
            ctx.fillStyle = Qt.rgba(0, 0, 0, 0.2);
            ctx.fillRect(0, 0, width, height)
            ctx.fillStyle = Qt.rgba(0, 0, 0, 0);
            ctx.globalCompositeOperation = "copy"
            ctx.strokeStyle = pane.valid ? pane.lineColor : pane.invalidLineColor
            ctx.beginPath()
            ctx.moveTo(topleft.center.x, topleft.center.y)
            ctx.lineTo(topright.center.x, topright.center.y)
            ctx.lineTo(bottomright.center.x, bottomright.center.y)
            ctx.lineTo(bottomleft.center.x, bottomleft.center.y)
            ctx.closePath()
            ctx.fill()
            ctx.stroke()
        }
    }

    DropArea {
        id: dropTarget
        anchors.fill: parent
        onDropped: {
            drop.source.x = drop.x
            drop.source.y = drop.y
        }
    }

    CornerMarker {
        id: topleft
        color: pane.markerColor
        x: (pane.width  - image.paintedWidth) / 2 + 50 - markerRadius
        y: (pane.height - image.paintedHeight) / 2 + 50 - markerRadius
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(x, y)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(x, y) }
    }

    CornerMarker {
        id: topright
        color: pane.markerColor
        x: (pane.width  + image.paintedWidth) / 2 - 50 - markerRadius
        y: (pane.height - image.paintedHeight) / 2 + 50 - markerRadius
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(x, y)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(x, y) }
    }

    CornerMarker {
        id: bottomleft
        color: pane.markerColor
        x: (pane.width  - image.paintedWidth) / 2 + 50 - markerRadius
        y: (pane.height + image.paintedHeight) / 2 - 50 - markerRadius
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(x, y)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(x, y) }
    }

    CornerMarker {
        id: bottomright
        color: pane.markerColor
        x: (pane.width  + image.paintedWidth) / 2 - 50 - markerRadius
        y: (pane.height + image.paintedHeight) / 2 - 50 - markerRadius
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(x, y)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(x, y) }
    }

    ZoomImage {
        id: zoomimg

        image: image.image
        filter: image.filterType

        borderColor: pane.markerColor
        crossColor: pane.lineColor

        width: Math.min(parent.width, parent.height) / 4
        height: Math.min(parent.width, parent.height) / 4

        viewSize: Qt.point(2.0 * markerRadius / image.paintedWidth, 2.0 * markerRadius / image.paintedHeight)

        anchors.left: pane.left
        anchors.top: pane.top
        anchors.margins: Math.min(parent.width, parent.height) / 20

        visible: false
    }

    function update(x, y) {
        pane.valid = Fotokopierer.isConvex(
            mapPoint(topleft.center),
            mapPoint(topright.center),
            mapPoint(bottomright.center),
            mapPoint(bottomleft.center))
        zoomimg.center = mapPoint(Qt.point(x + markerRadius, y + markerRadius))

        if (x < image.width / 2) {
            zoomimg.anchors.left = undefined
            zoomimg.anchors.right = pane.right
        } else {
            zoomimg.anchors.right = undefined
            zoomimg.anchors.left = pane.left
        }

        if (y < image.height / 2) {
            zoomimg.anchors.top = undefined
            zoomimg.anchors.bottom = pane.bottom
        } else {
            zoomimg.anchors.bottom = undefined
            zoomimg.anchors.top = pane.top
        }

        frame.requestPaint()
    }

    function mapPoint(p) {
        var x = (p.x - (pane.width - image.paintedWidth) / 2) / image.paintedWidth
        var y = (p.y - (pane.height - image.paintedHeight) / 2) / image.paintedHeight
        return Qt.point(x, y)
    }
}
