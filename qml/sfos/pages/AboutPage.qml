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

import QtQuick 2.6
import Sailfish.Silica 1.0
import Fotokopierer 1.0

Page {
    SilicaFlickable {
        anchors.fill: parent
        contentHeight: column.height + Theme.paddingLarge

        Column {
            id: column

            anchors.horizontalCenter: parent.horizontalCenter
            width: parent.width - Theme.paddingLarge * 2
            spacing: Theme.paddingLarge

            Label {
                id: apptitle

                width: parent.width

                font.pixelSize: Theme.fontSizeLarge
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: "%1 %2".arg(Fotokopierer.ApplicationName).arg(Fotokopierer.ApplicationVersion)
            }

            Label {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: qsTr("A camera scanning application for SailfishOS by %1")
                    .arg(Fotokopierer.Author)
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
            }

            Label {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: qsTr("This application uses\nPoDoFo %1\nOpenCV %2")
                    .arg(Fotokopierer.PoDoFoVersion)
                    .arg(Fotokopierer.OpenCVVersion)
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
            }

            Label {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: qsTr("Licensed under the %1").arg(Fotokopierer.LicenseTitle)
            }
        }
    }
}
