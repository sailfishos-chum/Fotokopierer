import QtQuick 2.0
import Sailfish.Silica 1.0

import "../../common"

Page {
	 id: page

	 SilicaFlickable {
		  anchors.fill: parent

		  CutImage {
				id: img
				anchors.fill: parent
		  }
	 }
}
