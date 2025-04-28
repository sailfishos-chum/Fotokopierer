/*
 * Copyright (c) 2018-2021 Frank Fischer <frank-fischer@shadow-soft.de>
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

    property bool dontchange: false

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
        var tl = mapPoint(topleft.markerPos)
        var tr = mapPoint(topright.markerPos)
        var br = mapPoint(bottomright.markerPos)
        var bl = mapPoint(bottomleft.markerPos)

        // compute and store the points of the rotated selection
        _next_tl = Qt.point(tr.y, 1-tr.x)
        _next_tr = Qt.point(br.y, 1-br.x)
        _next_br = Qt.point(bl.y, 1-bl.x)
        _next_bl = Qt.point(tl.y, 1-tl.x)

        image.filter.orientation -= 1
    }

    function rotateRight() {
        var tl = mapPoint(topleft.markerPos)
        var tr = mapPoint(topright.markerPos)
        var br = mapPoint(bottomright.markerPos)
        var bl = mapPoint(bottomleft.markerPos)

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

        var tl = Qt.point(tl.x * w + offx, tl.y * h + offy)
        var tr = Qt.point(tr.x * w + offx, tr.y * h + offy)
        var br = Qt.point(br.x * w + offx, br.y * h + offy)
        var bl = Qt.point(bl.x * w + offx, bl.y * h + offy)

        topleft.setCenter(tl)
        topright.setCenter(tr)
        bottomright.setCenter(br)
        bottomleft.setCenter(bl)

        top.setCenter(Qt.point(0.5 * (tl.x + tr.x), 0.5 * (tl.y + tr.y)))
        bottom.setCenter(Qt.point(0.5 * (bl.x + br.x), 0.5 * (bl.y + br.y)))
        left.setCenter(Qt.point(0.5 * (tl.x + bl.x), 0.5 * (tl.y + bl.y)))
        right.setCenter(Qt.point(0.5 * (tr.x + br.x), 0.5 * (tr.y + br.y)))
    }

    function cutImage() {
        var f = Scanner.cutFilter
        f.topLeft = mapPoint(topleft.markerPos)
        f.topRight = mapPoint(topright.markerPos)
        f.bottomRight = mapPoint(bottomright.markerPos)
        f.bottomLeft = mapPoint(bottomleft.markerPos)
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
            ctx.moveTo(topleft.markerPos.x, topleft.markerPos.y)
            ctx.lineTo(topright.markerPos.x, topright.markerPos.y)
            ctx.lineTo(bottomright.markerPos.x, bottomright.markerPos.y)
            ctx.lineTo(bottomleft.markerPos.x, bottomleft.markerPos.y)
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
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.topLeft = mapPoint(markerPos)
            top.setCenter(unmapPoint(Scanner.cutFilter.top))
            left.setCenter(unmapPoint(Scanner.cutFilter.left))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: topright
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.topRight = mapPoint(markerPos)
            top.setCenter(unmapPoint(Scanner.cutFilter.top))
            right.setCenter(unmapPoint(Scanner.cutFilter.right))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: bottomleft
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.bottomLeft = mapPoint(markerPos)
            bottom.setCenter(unmapPoint(Scanner.cutFilter.bottom))
            left.setCenter(unmapPoint(Scanner.cutFilter.left))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: bottomright
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.bottomRight = mapPoint(markerPos)
            bottom.setCenter(unmapPoint(Scanner.cutFilter.bottom))
            right.setCenter(unmapPoint(Scanner.cutFilter.right))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: top
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.top = mapPoint(markerPos)
            topleft.setCenter(unmapPoint(Scanner.cutFilter.topLeft))
            topright.setCenter(unmapPoint(Scanner.cutFilter.topRight))
            left.setCenter(unmapPoint(Scanner.cutFilter.left))
            right.setCenter(unmapPoint(Scanner.cutFilter.right))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                top.setCenter(unmapPoint(Scanner.cutFilter.top))
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: bottom
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.bottom = mapPoint(markerPos)
            bottomleft.setCenter(unmapPoint(Scanner.cutFilter.bottomLeft))
            bottomright.setCenter(unmapPoint(Scanner.cutFilter.bottomRight))
            left.setCenter(unmapPoint(Scanner.cutFilter.left))
            right.setCenter(unmapPoint(Scanner.cutFilter.right))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                bottom.setCenter(unmapPoint(Scanner.cutFilter.bottom))
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: left
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.left = mapPoint(markerPos)
            topleft.setCenter(unmapPoint(Scanner.cutFilter.topLeft))
            bottomleft.setCenter(unmapPoint(Scanner.cutFilter.bottomLeft))
            top.setCenter(unmapPoint(Scanner.cutFilter.top))
            bottom.setCenter(unmapPoint(Scanner.cutFilter.bottom))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                left.setCenter(unmapPoint(Scanner.cutFilter.left))
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: right
        color: pane.markerColor
        radius: markerRadius
        onDragged: {
            Scanner.cutFilter.right = mapPoint(markerPos)
            topright.setCenter(unmapPoint(Scanner.cutFilter.topRight))
            bottomright.setCenter(unmapPoint(Scanner.cutFilter.bottomRight))
            top.setCenter(unmapPoint(Scanner.cutFilter.top))
            bottom.setCenter(unmapPoint(Scanner.cutFilter.bottom))
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                right.setCenter(unmapPoint(Scanner.cutFilter.right))
                Scanner.cutFilter.fixSnappyEdges()
            }
        }
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
            mapPoint(topleft.markerPos),
            mapPoint(topright.markerPos),
            mapPoint(bottomright.markerPos),
            mapPoint(bottomleft.markerPos))
        zoomimg.center = mapPoint(zoompoint)

        if (zoompoint.x < image.width / 2) {
            zoomimg.anchors.left = undefined
            zoomimg.anchors.right = pane.right
        } else {
            zoomimg.anchors.right = undefined
            zoomimg.anchors.left = pane.left
        }

        if (zoompoint.y < image.height / 2) {
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

    function unmapPoint(p) {
        var x = p.x * image.paintedWidth + (pane.width - image.paintedWidth) / 2
        var y = p.y * image.paintedHeight + (pane.height - image.paintedHeight) / 2
        return Qt.point(x, y)
    }

    function selectionFromFilter() {
        _selectPoints(Scanner.cutFilter.topLeft,
                      Scanner.cutFilter.topRight,
                      Scanner.cutFilter.bottomRight,
                      Scanner.cutFilter.bottomLeft)

    }
}
