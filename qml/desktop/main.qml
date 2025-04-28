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
import QtQuick.Controls 1.2
import QtQuick.Layouts 1.0
import Fotokopierer 1.0
import "../common"

ApplicationWindow {
	 visible: true
	 title: "Fotokopierer"

	 width: 400
	 height: 600

	 ScannedImage {
		  id: img
	 }

	 Item {
		  id: imagebox
		  anchors.top: parent.top
		  anchors.left: parent.left
		  anchors.right: parent.right
		  anchors.bottom: button.top

		  CutImage {
				id: image
				anchors.fill: parent

				visible: true
		  }

		  Image {
	 	  		id: cutimage
	 	  		anchors.fill: parent
	 	  		fillMode: Image.PreserveAspectFit

	 	  		source: "image://Scanned/" + img.cut
				cache: false

	 	  		visible: false
		  }
	 }

	 Button {
		  id: button
		  anchors.left: parent.left
		  anchors.right: parent.right
		  anchors.bottom: parent.bottom
		  text: "Ok"
		  onClicked: {
				if (image.visible) {
					 cutimage.source = ""
					 cutimage.source = "image://Scanned/" + img.cut
				}
				image.visible = !image.visible
				cutimage.visible = !cutimage.visible
		  }
	 }


}
