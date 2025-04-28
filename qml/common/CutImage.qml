import QtQuick 2.0
import QtQuick.Layouts 1.0
import Fotokopierer 1.0

Item {
	 id: pane

	 property rect cutrect: Qt.rect(10, 10, 50, 100)

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
				ctx.strokeStyle = "#00FF00"
				var x = (pane.width - image.paintedWidth) / 2 + pane.cutrect.top
				var y = (pane.height - image.paintedHeight) / 2 + pane.cutrect.left
				var w = pane.cutrect.width
				var h = pane.cutrect.height
				ctx.strokeRect(x, y, w, h)
				ctx.strokeStyle = "#FFFFFF"
				ctx.ellipse(x-5,   y-5,   10, 10)
				ctx.ellipse(x+w-5, y-5,   10, 10)
				ctx.ellipse(x-5,   y+h-5, 10, 10)
				ctx.ellipse(x+w-5, y+h-5, 10, 10)
				ctx.stroke()
		  }
	 }
}
