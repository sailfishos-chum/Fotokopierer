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
            width: parent.width - Theme.horizontalPageMargin * 2
            spacing: Theme.paddingLarge

            PageHeader {
                title: qsTr("About %1").arg(Fotokopierer.ApplicationName)
            }

            Image {
                fillMode: Image.PreserveAspectFit
                source: Qt.resolvedUrl("/icons/harbour-fotokopierer.svg")
                width: Math.min(2/3 * parent.width, 2/3 * parent.height)
                height: width
                anchors.horizontalCenter: parent.horizontalCenter
            }

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

            Label {
                width: parent.width

                anchors.topMargin: Theme.fontSizeLarge
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: (qsTr("Icons by %1\nCopy icon by %2 of www.flaticon.com\nCut icon by %3 of www.flaticon.com")
                       .arg("Tobias Planitzer")
                       .arg("Iconnice")
                       .arg("Freepik"))

                onLinkActivated: Qt.openUrlExternally(link)
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
            }

            Label {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: ("<a href=\"http://chiselapp.com/user/fifr/repository/fotokopierer\">" +
                       qsTr("Homepage") + "</a>")

                onLinkActivated: Qt.openUrlExternally(link)
            }

            Label {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap

                text: (qsTr("Find me on Freenode: ") + 
                       "<a href=\"https://kiwiirc.com/nextclient/irc.freenode.net/#fotokopierer\">" +
                       "#fotokopierer")

                onLinkActivated: Qt.openUrlExternally(link)
            }

            Separator {
                width: parent.width
                horizontalAlignment: Qt.AlignHCenter
            }

            Label {
                width: parent.width

                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.WordWrap
                textFormat: Text.RichText

                text: (qsTr("This application uses") +
                       "<br><a href=\"http://podofo.sourceforge.net\">PoDoFo %1</a>" +
                       "<br><a href=\"https://opencv.org\">OpenCV %2</a>")
                    .arg(Fotokopierer.PoDoFoVersion)
                    .arg(Fotokopierer.OpenCVVersion)

                onLinkActivated: Qt.openUrlExternally(link)
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
