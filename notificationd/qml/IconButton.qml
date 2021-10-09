/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import FishUI 1.0 as FishUI

Item {
    id: control

    property url source
    property real size: 30
    property string popupText

    signal leftButtonClicked
    signal rightButtonClicked

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: control.visible ? true : false

        onClicked: {
            if (mouse.button === Qt.LeftButton)
                control.leftButtonClicked()
            else if (mouse.button === Qt.RightButton)
                control.rightButtonClicked()
        }
    }

    Rectangle {
        anchors.fill: parent
        anchors.margins: 1
        // radius: parent.height * 0.2
        radius: parent.height / 2

        color: {
            if (mouseArea.containsMouse) {
                if (mouseArea.containsPress)
                    return (FishUI.Theme.darkMode) ? Qt.rgba(255, 255, 255, 0.3) : Qt.rgba(0, 0, 0, 0.2)
                else
                    return (FishUI.Theme.darkMode) ? Qt.rgba(255, 255, 255, 0.2) : Qt.rgba(0, 0, 0, 0.1)
            }

            return "transparent"
        }
    }

    Image {
        id: iconImage
        anchors.centerIn: parent
        width: parent.height * 0.8
        height: width
        sourceSize.width: width
        sourceSize.height: height
        source: control.source
        asynchronous: true
        antialiasing: true
        smooth: false
    }
}
