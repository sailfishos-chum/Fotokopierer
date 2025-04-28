import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import Fotokopierer 1.0
import "../common"

ApplicationWindow {
	 visible: true
	 title: "Fotokopierer"

	 ScannedImage {
		  id: img
	 }

	 ColumnLayout {
		  anchors.fill: parent

		  CutImage {
				id: image
				anchors.fill: parent
		  }
	 }
}
