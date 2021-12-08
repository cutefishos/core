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
import QtGraphicalEffects 1.0

import Cutefish.Accounts 1.0 as Accounts
import Cutefish.System 1.0 as System
import FishUI 1.0 as FishUI

ApplicationWindow {
    width: Screen.width
    height: Screen.height
    visible: true
    visibility: Window.FullScreen
    flags: Qt.FramelessWindowHint | Qt.X11BypassWindowManagerHint
    id: root

    function exit() {
        Qt.quit()
    }

    System.Wallpaper {
        id: wallpaper
    }

    Rectangle {
        anchors.fill: parent
        color: wallpaper.color
        visible: wallpaper.type === 1
    }

    Image {
        id: wallpaperImage
        anchors.fill: parent
        source: "file://" + wallpaper.path
        sourceSize: Qt.size(width * Screen.devicePixelRatio,
                            height * Screen.devicePixelRatio)
        fillMode: Image.PreserveAspectCrop
        asynchronous: false
        clip: true
        cache: false
        smooth: false
        visible: wallpaper.type === 0
    }

    FastBlur {
        id: wallpaperBlur
        anchors.fill: parent
        radius: 64
        source: wallpaperImage
        cached: true
        visible: wallpaperImage.visible
    }

    ColorOverlay {
        anchors.fill: parent
        source: parent
        color: "#000000"
        opacity: 0.2
    }

    Accounts.UserAccount {
        id: currentUser
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

        Keys.onEnterPressed: {
            buttonsItem.action()
        }

        Keys.onReturnPressed: {
            buttonsItem.action()
        }

        Keys.onLeftPressed: {
            if (buttonsItem.currentIndex === -1 || buttonsItem.currentIndex === 0) {
                buttonsItem.currentIndex = 0
            } else {
                buttonsItem.currentIndex = buttonsItem.currentIndex - 1
            }

            buttonsItem.updateChecked()
        }

        Keys.onRightPressed: {
            if (buttonsItem.currentIndex >= 4) {

            } else {
                buttonsItem.currentIndex = buttonsItem.currentIndex + 1
            }

            buttonsItem.updateChecked()
        }

        Item {
            id: userItem
            anchors.top: parent.top
            anchors.bottom: buttonsItem.top
            anchors.bottomMargin: root.height * 0.05
            anchors.left: parent.left
            anchors.right: parent.right

            ColumnLayout {
                anchors.fill: parent
                spacing: FishUI.Units.largeSpacing

                Item {
                    Layout.fillHeight: true
                }

                Image {
                    id: userIcon

                    property int iconSize: 60

                    Layout.preferredHeight: iconSize
                    Layout.preferredWidth: iconSize
                    sourceSize: String(source) === "image://icontheme/default-user" ? Qt.size(iconSize, iconSize) : undefined
                    source: currentUser.iconFileName ? "file:///" + currentUser.iconFileName : "image://icontheme/default-user"
                    Layout.alignment: Qt.AlignHCenter

                    layer.enabled: true
                    layer.effect: OpacityMask {
                        maskSource: Item {
                            width: userIcon.width
                            height: userIcon.height

                            Rectangle {
                                anchors.fill: parent
                                radius: parent.height / 2
                            }
                        }
                    }

                    // No Action
                    MouseArea {
                        anchors.fill: parent
                    }
                }

                Label {
                    Layout.alignment: Qt.AlignHCenter
                    text: currentUser.userName
                    color: "white"

                    // No Action
                    MouseArea {
                        anchors.fill: parent
                    }
                }
            }
        }

        Item {
            id: buttonsItem
            anchors.centerIn: parent
            width: buttonsLayout.implicitWidth
            height: buttonsLayout.implicitHeight

            property int currentIndex: -1

            function updateChecked() {
                shutdownButton.checked = currentIndex === 0
                rebootButton.checked = currentIndex === 1
                logoutButton.checked = currentIndex === 2
                lockscreenButton.checked = currentIndex === 3
                suspendButton.checked = currentIndex === 4
            }

            function action() {
                if (buttonsItem.currentIndex === -1 || buttonsItem.currentIndex > 4)
                    return

                switch (currentIndex) {
                case 0:
                    actions.shutdown()
                    break
                case 1:
                    actions.reboot()
                    break
                case 2:
                    actions.logout()
                    break
                case 3:
                    actions.lockScreen()
                    break
                case 4:
                    actions.suspend()
                    break
                }
            }

            RowLayout {
                id: buttonsLayout
                anchors.fill: parent
                spacing: root.width * 0.015

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
            }
        }
    }
}
