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

import QtQuick 2.12
import QtQuick.Window 2.3
import QtQuick.Controls 2.5
import QtQuick.Layouts 1.3
import FishUI 1.0 as FishUI

ApplicationWindow {
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: Window.FullScreen
    flags: Qt.FramelessWindowHint | Qt.X11BypassWindowManagerHint
    id: root

    color: "transparent"

    function exit() {
        Qt.quit()
    }

    background: Rectangle {
        color: "black"
        opacity: 0.7
    }

    FishUI.WindowBlur {
        view: root
        geometry: Qt.rect(root.x, root.y, root.width, root.height)
        windowRadius: 0
        enabled: true
    }

    onActiveChanged: {
        if (!active)
            exit()
    }

    MouseArea {
        anchors.fill: parent
        onClicked: exit()
    }

    Item {
        id: rootItem
        anchors.fill: parent
        focus: true

        Keys.enabled: true
        Keys.onEscapePressed: {
            Qt.quit()
        }

        RowLayout {
            id: layout
            anchors.fill: parent
            spacing: root.width * 0.05

            Item {
                Layout.fillWidth: true
            }

            IconButton {
                id: shutdownButton
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Shutdown")
                icon: "qrc:///icons/system-shutdown.svg"
                onClicked: actions.shutdown()
            }

            IconButton {
                id: rebootButton
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Reboot")
                icon: "qrc:///icons/system-reboot.svg"
                onClicked: actions.reboot()
            }

            IconButton {
                id: logoutButton
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Logout")
                icon: "qrc:///icons/system-log-out.svg"
                onClicked: actions.logout()
            }

            IconButton {
                id: lockscreenButton
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Lock screen")
                icon: "qrc:/icons/system-lock-screen.svg"
                onClicked: actions.lockScreen()
            }

            IconButton {
                id: suspendButton
                Layout.alignment: Qt.AlignVCenter
                text: qsTr("Suspend")
                icon: "qrc:///icons/system-suspend.svg"
                onClicked: actions.suspend()
            }

            Item {
                Layout.fillWidth: true
            }
        }
    }
}
