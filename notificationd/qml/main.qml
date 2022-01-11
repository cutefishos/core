/*
    SPDX-FileCopyrightText: 2021 Reion Wong <reion@cutefishos.com>
    SPDX-FileCopyrightText: 2019 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

import QtQuick 2.12
import QtQml 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Window 2.12
import QtGraphicalEffects 1.0
import FishUI 1.0 as FishUI
import Cutefish.Notification 1.0

Item {
    id: root

    width: 0
    height: 0
    visible: false

    property int popupEdgeDistance: FishUI.Units.largeSpacing
    property int popupSpacing: FishUI.Units.largeSpacing
    readonly property real popupMaximumScreenFill: 0.4

    readonly property rect screenRect: {
        let rect = Qt.rect(screen.screenGeometry.x + screen.availableScreenRect.x,
                           screen.screenGeometry.y + screen.availableScreenRect.y,
                           screen.availableScreenRect.width,
                           screen.availableScreenRect.height)
        return rect
    }

    onScreenRectChanged: positionPopups()

    property Instantiator popupInstantiator: Instantiator {
        model: notificationsModel
        delegate: QmlNotificationPopup {}

        onObjectAdded: {
            positionPopups()
        }

        onObjectRemoved: {
            positionPopups()
        }
    }

    ScreenHelper {
        id: screen
    }

    function positionPopups() {
        const screenRect = root.screenRect

        if (screenRect.width <= 0 || screenRect.height <= 0) {
            return
        }

        let y = screenRect.y
        y += root.popupEdgeDistance

        for (var i = 0; i < popupInstantiator.count; ++i) {
            let popup = popupInstantiator.objectAt(i)

            if (!popup)
                continue

            var popupEffectiveWidth = popup.width
            const leftMostX = screenRect.x + root.popupEdgeDistance
            const rightMostX = screenRect.x + screenRect.width - root.popupEdgeDistance - popupEffectiveWidth

            // right
            popup.x = rightMostX
            popup.y = y
            y += popup.height + (popup.height > 0 ? popupSpacing : 0)

            // Horizontal
            // popup.x = screenRect.x + (screenRect.width - popup.width) / 2

            // don't let notifications take more than popupMaximumScreenFill of the screen
            var visible = true
            if (i > 0) { // however always show at least one popup
                visible = (popup.y + popup.height < screenRect.y + (screenRect.height * popupMaximumScreenFill))
            }

            popup.visible = visible
        }
    }
}
