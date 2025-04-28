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
import QtQuick.Layouts 1.0
import QtGraphicalEffects 1.0
import Fotokopierer 1.0

Item {
	 id: pane

	 property real markerRadius: 10

	 ScannedImage {
		  id: img
	 }

	 Image {
		  id: image
		  anchors.fill: parent
		  fillMode: Image.PreserveAspectFit
		  source: "image://Scanned/" + img.original
	 }

	 Canvas {
		  id: frame
		  anchors.fill: parent
		  onPaint: {
				var ctx = getContext("2d")
				ctx.clearRect(0, 0, width, height)
				ctx.strokeStyle = "#00FF00"
				ctx.beginPath()
				ctx.moveTo(topleft.center.x, topleft.center.y)
				ctx.lineTo(topright.center.x, topright.center.y)
				ctx.lineTo(bottomright.center.x, bottomright.center.y)
				ctx.lineTo(bottomleft.center.x, bottomleft.center.y)
				ctx.closePath()
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
	 	  x: (pane.width  - image.paintedWidth) / 2 + 10 - markerRadius
	 	  y: (pane.height - image.paintedHeight) / 2 + 10 - markerRadius
		  minX: (pane.width - image.paintedWidth) / 2 - markerRadius
		  maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
		  minY: (pane.height - image.paintedHeight) / 2 - markerRadius
		  maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }

	 ZoomImage {
		  image: image

		  anchors.right: pane.right
		  anchors.bottom: pane.bottom
		  anchors.leftMargin: 5
		  anchors.rightMargin: 5
		  anchors.topMargin: 5
		  anchors.bottomMargin: 5

		  imagex: topleft.x
		  imagey: topleft.y

		  visible: topleft.dragActive
	 }

	 CornerMarker {
		  id: topright
	 	  x: (pane.width  + image.paintedWidth) / 2 - 10 - markerRadius
	 	  y: (pane.height - image.paintedHeight) / 2 + 10 - markerRadius
		  minX: (pane.width - image.paintedWidth) / 2 - markerRadius
		  maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
		  minY: (pane.height - image.paintedHeight) / 2 - markerRadius
		  maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }

	 ZoomImage {
		  image: image

		  anchors.left: pane.left
		  anchors.bottom: pane.bottom
		  anchors.leftMargin: 5
		  anchors.rightMargin: 5
		  anchors.topMargin: 5
		  anchors.bottomMargin: 5

		  imagex: topright.x
		  imagey: topright.y

		  visible: topright.dragActive
	 }

	 CornerMarker {
		  id: bottomleft
	 	  x: (pane.width  - image.paintedWidth) / 2 + 10 - markerRadius
	 	  y: (pane.height + image.paintedHeight) / 2 - 10 - markerRadius
		  minX: (pane.width - image.paintedWidth) / 2 - markerRadius
		  maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
		  minY: (pane.height - image.paintedHeight) / 2 - markerRadius
		  maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
		  radius: markerRadius
		  onCenterChanged: {
				frame.requestPaint()
		  }
	 }

	 ZoomImage {
		  image: image

		  anchors.right: pane.right
		  anchors.top: pane.top
		  anchors.leftMargin: 5
		  anchors.rightMargin: 5
		  anchors.topMargin: 5
		  anchors.bottomMargin: 5

		  imagex: bottomleft.x
		  imagey: bottomleft.y

		  visible: bottomleft.dragActive
	 }

	 CornerMarker {
		  id: bottomright
	 	  x: (pane.width  + image.paintedWidth) / 2 - 10- markerRadius
	 	  y: (pane.height + image.paintedHeight) / 2 - 10 - markerRadius
		  minX: (pane.width - image.paintedWidth) / 2 - markerRadius
		  maxX: (pane.width + image.paintedWidth) / 2 - markerRadius
		  minY: (pane.height - image.paintedHeight) / 2 - markerRadius
		  maxY: (pane.height + image.paintedHeight) / 2 - markerRadius
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }

	 ZoomImage {
		  image: image

		  anchors.left: pane.left
		  anchors.top: pane.top
		  anchors.leftMargin: 5
		  anchors.rightMargin: 5
		  anchors.topMargin: 5
		  anchors.bottomMargin: 5

		  imagex: bottomright.x
		  imagey: bottomright.y

		  visible: bottomright.dragActive
	 }
}
