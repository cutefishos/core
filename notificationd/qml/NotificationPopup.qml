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
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import FishUI 1.0 as FishUI
import Cutefish.Notification 1.0

Window {
    id: control
    flags: Qt.WindowDoesNotAcceptFocus | Qt.FramelessWindowHint | Qt.WindowStaysOnTopHint | Qt.Popup
    width: 400
    height: 70
    color: "transparent"
    visible: false
    onVisibleChanged: if (visible) timer.restart()

    property int defaultTimeout: 7000

    FishUI.WindowShadow {
        view: control
        radius: _background.radius
    }

    FishUI.WindowBlur {
        view: control
        geometry: Qt.rect(control.x, control.y, control.width, control.height)
        windowRadius: _background.radius
        enabled: true
    }

    Rectangle {
        id: _background
        anchors.fill: parent
        radius: height * 0.2
        color: FishUI.Theme.backgroundColor
        opacity: 0.5
    }

    MouseArea {
        id: _mouseArea
        z: 999
        anchors.fill: parent
        onClicked: notificationsModel.close(model.notificationId)
        hoverEnabled: true
        onEntered: timer.stop()
        onExited: timer.restart()
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: FishUI.Units.largeSpacing
        anchors.rightMargin: FishUI.Units.smallSpacing
        anchors.topMargin: FishUI.Units.smallSpacing
        anchors.bottomMargin: FishUI.Units.smallSpacing
        spacing: FishUI.Units.largeSpacing

        Image {
            id: _icon
            width: 48
            height: width
            source: "image://icontheme/%1".arg(model.iconName)
            sourceSize: Qt.size(width, height)
            Layout.alignment: Qt.AlignVCenter
        }

        ColumnLayout {
            spacing: 0

            Item {
                Layout.fillHeight: true
            }

            Label {
                text: model.summary
                visible: text
                elide: Text.ElideRight
                Layout.fillWidth: true
                rightPadding: FishUI.Units.smallSpacing
            }

            Label {
                text: model.body
                visible: text
                rightPadding: FishUI.Units.smallSpacing
                maximumLineCount: 2
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                Layout.fillWidth: true
                // Layout.fillHeight: true
                Layout.alignment: Qt.AlignVCenter
            }

            Item {
                Layout.fillHeight: true
            }
        }
    }

    Image {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.topMargin: FishUI.Units.smallSpacing / 2
        anchors.rightMargin: FishUI.Units.smallSpacing
        width: 24
        height: 24
        source: "qrc:/images/" + (FishUI.Theme.darkMode ? "dark" : "light") + "/close.svg"
        sourceSize: Qt.size(width, height)
        visible: _mouseArea.containsMouse
    }

    Timer {
        id: timer
        interval: control.defaultTimeout

        onTriggered: {
            notificationsModel.close(model.notificationId)
        }
    }
}
