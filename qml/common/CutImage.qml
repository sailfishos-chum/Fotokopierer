import QtQuick 2.0
import QtQuick.Layouts 1.0
import Fotokopierer 1.0

Item {
	 id: pane

	 property rect cutrect: Qt.rect(10, 10, 50, 100)

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
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }

	 CornerMarker {
		  id: topright
	 	  x: (pane.width  - image.paintedWidth) / 2 + 50 - markerRadius
	 	  y: (pane.height - image.paintedHeight) / 2 + 10 - markerRadius
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }

	 CornerMarker {
		  id: bottomleft
	 	  x: (pane.width  - image.paintedWidth) / 2 + 10 - markerRadius
	 	  y: (pane.height - image.paintedHeight) / 2 + 100 - markerRadius
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }

	 CornerMarker {
		  id: bottomright
	 	  x: (pane.width  - image.paintedWidth) / 2 + 50 - markerRadius
	 	  y: (pane.height - image.paintedHeight) / 2 + 100 - markerRadius
		  radius: markerRadius
		  onCenterChanged: frame.requestPaint()
	 }
}
