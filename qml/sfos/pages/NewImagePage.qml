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
    property Page destination

    signal addPage(PlainImage original, ColorizeImage result)

    // Note that this property might become unsupported in future
    popOnSelection: false

    PlainImage { id: plain }

    CutPage { id: cutpage; source: plain }

    ColorizePage {
        id: colpage
        source: cutpage.image
        acceptDestination: destination
        acceptDestinationAction: PageStackAction.Pop

        onAccepted: {
            addPage(plain, colpage.image)
        }
    }

    onSelectedContentPropertiesChanged: {
        plain.loadFile(selectedContentProperties.filePath)
        pageStack.push(cutpage)
        pageStack.pushAttached(colpage)
    }
}
