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
        }
    }

    function rotateLeft() {
        image.filter.orientation -= 1
        Scanner.cutFilter.rotateLeft()
        frame.requestPaint()
    }

    function rotateRight() {
        image.filter.orientation += 1
        Scanner.cutFilter.rotateRight()
        frame.requestPaint()
    }

    function selectAll() {
        var points = Scanner.cutFilter.selectAll()
        Scanner.cutFilter.fixSnappyEdges()
        frame.requestPaint()
    }

    function selectAuto() {
        var points = Scanner.cutFilter.autoDetectCutRect()
        Scanner.cutFilter.fixSnappyEdges()
        frame.requestPaint()
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.topLeft = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.topRight = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.bottomLeft = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.bottomRight = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.top = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.bottom = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.left = mapPoint(markerPos)
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
        minX: (pane.width - image.paintedWidth) / 2
        maxX: (pane.width + image.paintedWidth) / 2
        minY: (pane.height - image.paintedHeight) / 2
        maxY: (pane.height + image.paintedHeight) / 2
        onDragged: {
            Scanner.cutFilter.right = mapPoint(markerPos)
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

    Timer {
        id: initTimer
        interval: 1
        repeat: false
        onTriggered: selectAuto()
    }

    Component.onCompleted: {
        Scanner.cutFilter.topLeftChanged.connect(function() {
            topleft.setCenter(unmapPoint(Scanner.cutFilter.topLeft))
        })

        Scanner.cutFilter.topRightChanged.connect(function() {
            topright.setCenter(unmapPoint(Scanner.cutFilter.topRight))
        })

        Scanner.cutFilter.bottomRightChanged.connect(function() {
            bottomright.setCenter(unmapPoint(Scanner.cutFilter.bottomRight))
        })

        Scanner.cutFilter.bottomLeftChanged.connect(function() {
            bottomleft.setCenter(unmapPoint(Scanner.cutFilter.bottomLeft))
        })

        Scanner.cutFilter.topChanged.connect(function() {
            top.setCenter(unmapPoint(Scanner.cutFilter.top))
        })

        Scanner.cutFilter.bottomChanged.connect(function() {
            bottom.setCenter(unmapPoint(Scanner.cutFilter.bottom))
        })

        Scanner.cutFilter.leftChanged.connect(function() {
            left.setCenter(unmapPoint(Scanner.cutFilter.left))
        })

        Scanner.cutFilter.rightChanged.connect(function() {
            right.setCenter(unmapPoint(Scanner.cutFilter.right))
        })

        Scanner.originalImageChanged.connect(function() {
            initTimer.start()
        })
    }
}
