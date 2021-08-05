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
import FishUI 1.0 as FishUI

Item {
    id: control
    width: 128
    height: width

    property string text
    property string icon
    property bool checked: false
    signal clicked

    Rectangle {
        anchors.fill: parent
        color: "white"
        opacity: mouseArea.pressed || control.checked ? 0.1 : mouseArea.containsMouse ? 0.2 : 0
        radius: height / 2

        Behavior on opacity {
            NumberAnimation {
                duration: 100
            }
        }
    }

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
        spacing: FishUI.Units.smallSpacing

        Item {
            Layout.fillHeight: true
        }

        Image {
            id: image
            source: control.icon
            width: control.width * 0.5
            height: width
            sourceSize.width: width
            sourceSize.height: width
            Layout.alignment: Qt.AlignCenter
        }

        Label {
            Layout.alignment: Qt.AlignCenter
            text: control.text
            color: "white"
        }

        Item {
            Layout.fillHeight: true
        }
    }
}
