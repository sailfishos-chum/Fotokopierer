/*
 * Copyright (c) 2018, 2019 Frank Fischer <frank-fischer@shadow-soft.de>
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
import QtGraphicalEffects 1.0
import Sailfish.Silica 1.0

Item {
    property alias value: slider.value
    property alias icon: img.source

    anchors.left: parent.left
    anchors.right: parent.right
    height: slider.height

    Image {
        id: img
        width: Theme.iconSizeSmall
        height: Theme.iconSizeSmall
        anchors.left: parent.left
        anchors.leftMargin: Theme.iconSizeSmall
        anchors.rightMargin: Theme.iconSizeSmall
        anchors.verticalCenter: parent.verticalCenter
        source: icon
    }

    ColorOverlay {
        anchors.fill: img
        source: img
        color: Theme.highlightColor
    }

    Slider {
        id: slider
        anchors.left: img.right
        anchors.right: parent.right
        leftMargin: Theme.iconSizeSmall
        rightMargin: Theme.iconSizeSmall
        value: 50
        minimumValue: 0
        maximumValue: 100
    }
}
