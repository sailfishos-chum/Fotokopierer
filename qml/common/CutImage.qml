import QtQuick 2.0
import QtQuick.Layouts 1.0
import Fotokopierer 1.0
import "."

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
				ctx.moveTo(topleft.centerx, topleft.centery)
				ctx.lineTo(topright.centerx, topright.centery)
				ctx.lineTo(bottomright.centerx, bottomright.centery)
				ctx.lineTo(bottomleft.centerx, bottomleft.centery)
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
	 	  centerx: (pane.width  - image.paintedWidth) / 2 + 10
	 	  centery: (pane.height - image.paintedHeight) / 2 + 10
		  radius: markerRadius
		  onCenterxChanged: frame.requestPaint()
		  onCenteryChanged: frame.requestPaint()
	 }

	 CornerMarker {
		  id: topright
	 	  centerx: (pane.width  - image.paintedWidth) / 2 + 50
	 	  centery: (pane.height - image.paintedHeight) / 2 + 10
		  radius: markerRadius
		  onCenterxChanged: frame.requestPaint()
		  onCenteryChanged: frame.requestPaint()
	 }

	 CornerMarker {
		  id: bottomleft
	 	  centerx: (pane.width  - image.paintedWidth) / 2 + 10
	 	  centery: (pane.height - image.paintedHeight) / 2 + 100
		  radius: markerRadius
		  onCenterxChanged: frame.requestPaint()
		  onCenteryChanged: frame.requestPaint()
	 }

	 CornerMarker {
		  id: bottomright
	 	  centerx: (pane.width  - image.paintedWidth) / 2 + 50
	 	  centery: (pane.height - image.paintedHeight) / 2 + 100
		  radius: markerRadius
		  onCenterxChanged: frame.requestPaint()
		  onCenteryChanged: frame.requestPaint()
	 }
}
