/*
 * Copyright (c) 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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
import QtQuick.Layouts 1.0
import Sailfish.Silica 1.0
import Fotokopierer 1.0

Dialog {
    id: page

    property var document

    Column {
        width: parent.width

        DialogHeader {
            title: qsTr("Enter new document title")
        }

        TextField {
            id: docEdit
            width: parent.width
            label: qsTr("Document title")
            placeholderText: document.defaultTitle
            text: document.title
            focus: true
            validator: RegExpValidator { regExp: /(\w|\d|[- ():,.])*/ }
            EnterKey.onClicked: page.accept()
        }
    }

    Component.onCompleted: docEdit.selectAll()

    onAccepted: {
        if (docEdit.text == "") {
            document.title = document.defaultTitle
        } else {
            document.title = docEdit.text
        }
    }
}
