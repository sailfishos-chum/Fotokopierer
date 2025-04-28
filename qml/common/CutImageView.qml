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

    property alias busy: cutview.busy
    property alias hasAutoSelection: cutview.hasAutoSelection

    // The next properties are used to rotate the selection when the image has
    // been rotated. Because the computation of the rotated image is done
    // asynchronously, we must wait with the update until the rotation has been
    // completed.
    property var _next_tl
    property var _next_tr
    property var _next_br
    property var _next_bl

    property bool dontchange: false

    CutView {
        id: cutview

        scanner: Scanner

        anchors.fill: parent
    }

    function rotateLeft() {
        cutview.rotateLeft()
        frame.requestPaint()
    }

    function rotateRight() {
        cutview.rotateRight()
        frame.requestPaint()
    }

    function selectAll() {
        cutview.selectAll()
        frame.requestPaint()
    }

    function selectAuto() {
        cutview.selectAuto()
        frame.requestPaint()
    }

    function cutImage() {
        cutview.apply()
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
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.topLeft = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                cutview.updateSnappyEdges()
            }
        }
    }

    CornerMarker {
        id: topright
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.topRight = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                cutview.updateSnappyEdges();
            }
        }
    }

    CornerMarker {
        id: bottomleft
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.bottomLeft = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                cutview.updateSnappyEdges();
            }
        }
    }

    CornerMarker {
        id: bottomright
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.bottomRight = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                cutview.updateSnappyEdges();
            }
        }
    }

    CornerMarker {
        id: top
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.top = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                top.setCenter(unmapPoint(cutview.top))
                cutview.updateSnappyEdges();
            }
        }
    }

    CornerMarker {
        id: bottom
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.bottom = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                bottom.setCenter(unmapPoint(cutview.bottom))
                cutview.updateSnappyEdges();
            }
        }
    }

    CornerMarker {
        id: left
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.left = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                left.setCenter(unmapPoint(cutview.left))
                cutview.updateSnappyEdges();
            }
        }
    }

    CornerMarker {
        id: right
        color: pane.markerColor
        radius: markerRadius
        minX: (pane.width - cutview.paintedWidth) / 2
        maxX: (pane.width + cutview.paintedWidth) / 2
        minY: (pane.height - cutview.paintedHeight) / 2
        maxY: (pane.height + cutview.paintedHeight) / 2
        onDragged: {
            cutview.right = mapPoint(markerPos)
            pane.update(markerPos)
        }
        onDragActiveChanged: {
            zoomimg.visible = dragActive
            pane.update(markerPos)
            if (!dragActive) {
                // end of dragging -> reset this point to the middle of the edge
                right.setCenter(unmapPoint(cutview.right))
                cutview.updateSnappyEdges();
            }
        }
    }

    ZoomImage {
        id: zoomimg

        scanner: Scanner

        borderColor: pane.markerColor
        crossColor: pane.lineColor

        width: Math.min(parent.width, parent.height) / 4
        height: Math.min(parent.width, parent.height) / 4

        viewSize: Qt.point(2.0 * markerRadius / cutview.paintedWidth, 2.0 * markerRadius / cutview.paintedHeight)

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

        if (zoompoint.x < cutview.width / 2) {
            zoomimg.anchors.left = undefined
            zoomimg.anchors.right = pane.right
        } else {
            zoomimg.anchors.right = undefined
            zoomimg.anchors.left = pane.left
        }

        if (zoompoint.y < cutview.height / 2) {
            zoomimg.anchors.top = undefined
            zoomimg.anchors.bottom = pane.bottom
        } else {
            zoomimg.anchors.bottom = undefined
            zoomimg.anchors.top = pane.top
        }

        frame.requestPaint()
    }

    function initSelection() {
        topleft.setCenter(unmapPoint(cutview.topLeft))
        topright.setCenter(unmapPoint(cutview.topRight))
        bottomright.setCenter(unmapPoint(cutview.bottomRight))
        bottomleft.setCenter(unmapPoint(cutview.bottomLeft))
        top.setCenter(unmapPoint(cutview.top))
        bottom.setCenter(unmapPoint(cutview.bottom))
        left.setCenter(unmapPoint(cutview.left))
        right.setCenter(unmapPoint(cutview.right))
        cutview.updateSnappyEdges()
        frame.requestPaint()
    }

    function mapPoint(p) {
        var x = (p.x - (pane.width - cutview.paintedWidth) / 2) / cutview.paintedWidth
        var y = (p.y - (pane.height - cutview.paintedHeight) / 2) / cutview.paintedHeight
        return Qt.point(x, y)
    }

    function unmapPoint(p) {
        var x = p.x * cutview.paintedWidth + (pane.width - cutview.paintedWidth) / 2
        var y = p.y * cutview.paintedHeight + (pane.height - cutview.paintedHeight) / 2
        return Qt.point(x, y)
    }

    Timer {
        id: initTimer
        interval: 1
        repeat: false
        onTriggered: selectAuto()
    }

    Component.onCompleted: {
        cutview.topLeftChanged.connect(function() {
            topleft.setCenter(unmapPoint(cutview.topLeft))
        })

        cutview.topRightChanged.connect(function() {
            topright.setCenter(unmapPoint(cutview.topRight))
        })

        cutview.bottomRightChanged.connect(function() {
            bottomright.setCenter(unmapPoint(cutview.bottomRight))
        })

        cutview.bottomLeftChanged.connect(function() {
            bottomleft.setCenter(unmapPoint(cutview.bottomLeft))
        })

        cutview.topChanged.connect(function() {
            top.setCenter(unmapPoint(cutview.top))
        })

        cutview.bottomChanged.connect(function() {
            bottom.setCenter(unmapPoint(cutview.bottom))
        })

        cutview.leftChanged.connect(function() {
            left.setCenter(unmapPoint(cutview.left))
        })

        cutview.rightChanged.connect(function() {
            right.setCenter(unmapPoint(cutview.right))
        })

        cutview.rotationChanged.connect(function() {
            frame.requestPaint()
        })

        /* Scanner.originalImageChanged.connect(function() { */
        /*     initTimer.start() */
        /* }) */
    }
}
