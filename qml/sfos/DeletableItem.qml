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

import Sailfish.Silica 1.0
import QtQuick 2.0

Item {
    property bool deleting: false
    property alias deleteIcon: deleteButton.icon

    default property alias data: zoom.data

    signal deleteItem()

    Item {
        id: zoom

        width: parent.width * (deleting ? 0.9 : 1.0)
        height: parent.height * (deleting ? 0.9 : 1.0)

        anchors {
            horizontalCenter: parent.horizontalCenter
            verticalCenter: parent.verticalCenter
        }

        Behavior on width {
            NumberAnimation { duration: 100 }
        }

        Behavior on height {
            NumberAnimation { duration: 100 }
        }
    }

    IconButton {
        id: deleteButton

        visible: deleting
        anchors { top: parent.top; right: parent.right }
        icon.source: "image://theme/icon-l-clear"
        onClicked: deleteItem()
    }
}
