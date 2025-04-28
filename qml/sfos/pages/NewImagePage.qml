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
import Sailfish.Silica 1.0
import Sailfish.Pickers 1.0
import Fotokopierer 1.0

ImagePickerPage {
    signal addPage(PlainImage original, CutImage result)

    // Note that this property might become unsupported in future
    popOnSelection: false

    onSelectedContentPropertiesChanged: {
        var plain = Fotokopierer.loadPlainImage(selectedContentProperties.filePath)

        var CutPage = Qt.createComponent(Qt.resolvedUrl("CutPage.qml"))
        var cutpage = CutPage.createObject(docpage, {"source": plain})
        pageStack.push(cutpage)

        var ColorizePage = Qt.createComponent(Qt.resolvedUrl("ColorizePage.qml"))
        var colpage = ColorizePage.createObject(docpage,
                                                {
                                                    "source": cutpage.image,
                                                    "acceptDestination": docpage,
                                                    "acceptDestinationAction": PageStackAction.Pop,
                                                })
        pageContainer.pushAttached(colpage)
        colpage.accepted.connect(function() {
            document.addPage(plain, colpage.image)
        })

        // Ensure the pages are destroyed once they are dropped from the
        // page stack. This is necessary so that the memory allocated by
        // the image classes is freed (the memory is allocated on the
        // C++ side and possibly not visible for the QML garbage
        // collector).
        cutpage.pageContainerChanged.connect(function() {
            if (cutpage.pageContainer == null) {
                colpage.destroy()
                cutpage.destroy()
                plain.destroy()
            }
        })
    }
}
