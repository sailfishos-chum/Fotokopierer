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

import QtQuick 2.2
import Sailfish.Silica 1.0
import Fotokopierer 1.0
import "pages"

ApplicationWindow
{
    initialPage: Component { Documents { } }

    PlainImage {
        id: image
        visible: false
    }

    CutPage {
        id: cutpage
        onStatusChanged: {
            if (status == PageStatus.Active) {
                pageContainer.pushAttached(Qt.resolvedUrl("pages/ColorizePage.qml"), {source: cutpage.image})
            }
        }
    }

    Component.onCompleted: {
        console.log(Qt.application.arguments)
        if (Qt.application.arguments.length > 1) {
            console.log("FILE: " + Qt.application.arguments[1])
            image.loadFile(Qt.application.arguments[1])
            cutpage.source = image
        }
        //pageStack.pushAttached(Qt.resolvedUrl("pages/Documents.qml"))
        pageStack.pushAttached(cutpage)
    }
}
