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

    property real markerRadius: Math.min(width, height) / 25
    property color markerColor: "white"

    property color lineColor: "green"
    property color invalidLineColor: "red"

    property bool valid: true

    // The next properties are used to rotate the selection when the image has
    // been rotated. Because the computation of the rotated image is done
    // asynchronously, we must wait with the update until the rotation has been
    // completed.
    property var _next_tl
    property var _next_tr
    property var _next_br
    property var _next_bl

    FilterImage {
        id: image

        image: Scanner
        filterType: Scanner.Rotate

        anchors.fill: parent

        onPaintedSizeChanged: {
            // update the selection after a rotation has been completed
            if (_next_tl) {
                _selectPoints(_next_tl, _next_tr, _next_br, _next_bl)
                _next_tl = null
            }
        }
    }

    function rotateLeft() {
        var tl = mapPoint(topleft.center)
        var tr = mapPoint(topright.center)
        var br = mapPoint(bottomright.center)
        var bl = mapPoint(bottomleft.center)

        // compute and store the points of the rotated selection
        _next_tl = Qt.point(tr.y, 1-tr.x)
        _next_tr = Qt.point(br.y, 1-br.x)
        _next_br = Qt.point(bl.y, 1-bl.x)
        _next_bl = Qt.point(tl.y, 1-tl.x)

        image.filter.orientation -= 1
    }

    function rotateRight() {
        var tl = mapPoint(topleft.center)
        var tr = mapPoint(topright.center)
        var br = mapPoint(bottomright.center)
        var bl = mapPoint(bottomleft.center)

        // compute and store the points of the rotated selection
        _next_tl = Qt.point(1-bl.y, bl.x)
        _next_tr = Qt.point(1-tl.y, tl.x)
        _next_br = Qt.point(1-tr.y, tr.x)
        _next_bl = Qt.point(1-br.y, br.x)

        image.filter.orientation += 1
    }

    function selectAll() {
        _selectPoints(Qt.point(0, 0), Qt.point(1, 0), Qt.point(1, 1), Qt.point(0, 1))
    }

    function selectAuto() {
        var points = Scanner.cutFilter.autoDetectCutRect()
        _selectPoints(points[0], points[1], points[2], points[3])
    }

    function _selectPoints(tl, tr, br, bl) {
        var w = image.paintedWidth
        var h = image.paintedHeight
        var offx = (pane.width - w) / 2
        var offy = (pane.height - h) / 2
        topleft.setCenter(Qt.point(tl.x * w + offx, tl.y * h + offy))
        topright.setCenter(Qt.point(tr.x * w + offx, tr.y * h + offy))
        bottomright.setCenter(Qt.point(br.x * w + offx, br.y * h + offy))
        bottomleft.setCenter(Qt.point(bl.x * w + offx, bl.y * h + offy))
    }

    function cutImage() {
        var f = Scanner.cutFilter
        f.topLeft = mapPoint(topleft.center)
        f.topRight = mapPoint(topright.center)
        f.bottomRight = mapPoint(bottomright.center)
        f.bottomLeft = mapPoint(bottomleft.center)
        Scanner.cutFilter.updateCut()
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
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(center)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(center) }
    }

    CornerMarker {
        id: topright
        color: pane.markerColor
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(center)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(center) }
    }

    CornerMarker {
        id: bottomleft
        color: pane.markerColor
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(center)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(center) }
    }

    CornerMarker {
        id: bottomright
        color: pane.markerColor
        minX: (pane.width - image.paintedWidth) / 2 - markerRadius
        maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
        minY: (pane.height - image.paintedHeight) / 2 - markerRadius
        maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
        radius: markerRadius
        onCenterChanged: pane.update(center)
        onDragActiveChanged: { zoomimg.visible = dragActive; pane.update(center) }
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

    function update(zoompoint) {
        pane.valid = Fotokopierer.isConvex(
            mapPoint(topleft.center),
            mapPoint(topright.center),
            mapPoint(bottomright.center),
            mapPoint(bottomleft.center))
        zoomimg.center = mapPoint(zoompoint)

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

    function selectionFromFilter() {
        _selectPoints(Scanner.cutFilter.topLeft,
                      Scanner.cutFilter.topRight,
                      Scanner.cutFilter.bottomRight,
                      Scanner.cutFilter.bottomLeft)

    }
}
