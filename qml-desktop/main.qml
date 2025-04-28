import QtQuick 2.0
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0

ApplicationWindow {
	 visible: true
	 title: "Fotokopierer"

	 ColumnLayout {
		  anchors.fill: parent

		  Image {
				id: image
				anchors.fill: parent
				fillMode: Image.PreserveAspectFit
				source: "file:///home/fifr/Dokumente/Sonstiges/anja-see.jpg"
		  }
	 }
}
