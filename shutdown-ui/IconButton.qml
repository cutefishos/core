/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <aj@cutefishos.com>
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

import QtQuick 2.0
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import QtGraphicalEffects 1.0
import FishUI 1.0 as FishUI

Item {
    id: control
    width: 128
    height: width

    property string text
    property string icon
    property int iconSize: 52
    property bool checked: false
    signal clicked

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton
        onClicked: control.clicked()
    }

    ColumnLayout {
        id: layout
        anchors.fill: parent
        spacing: FishUI.Units.largeSpacing * 1.5

        Item {
            Layout.fillHeight: true
        }

        Item {
            height: control.iconSize
            Layout.fillWidth: true

            Rectangle {
                anchors.centerIn: parent
                width: parent.height + FishUI.Units.largeSpacing * 2
                height: parent.height + FishUI.Units.largeSpacing * 2
                z: -1
                color: "white"
                opacity: mouseArea.pressed ? 0.1 : mouseArea.containsMouse || control.checked ? 0.2 : 0
                radius: height / 2
            }

            Image {
                id: image
                anchors.centerIn: parent
                source: control.icon
                sourceSize: Qt.size(width, height)
                width: control.iconSize
                height: width
            }
        }

        Label {
            id: _label
            Layout.alignment: Qt.AlignCenter
            text: control.text
            color: "white"
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
