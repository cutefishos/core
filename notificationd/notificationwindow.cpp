/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <kate@cutefishos.com>
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

#include "notificationwindow.h"
#include "notificationsmodel.h"
#include "historymodel.h"

#include <QQmlContext>

#include <KWindowSystem>
#include <KWindowEffects>

NotificationWindow::NotificationWindow(QQuickView *parent)
    : QQuickView(parent)
{
    installEventFilter(this);

    setFlags(Qt::Popup);
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::transparent);

    rootContext()->setContextProperty("NotificationDialog", this);
    rootContext()->setContextProperty("notificationsModel", NotificationsModel::self());
    rootContext()->setContextProperty("historyModel", HistoryModel::self());

    // KWindowEffects::slideWindow(winId(), KWindowEffects::RightEdge);
    setSource(QUrl("qrc:/qml/NotificationWindow.qml"));
    setVisible(false);
}

void NotificationWindow::open()
{
    setVisible(true);
    setMouseGrabEnabled(true);
    setKeyboardGrabEnabled(true);
}

bool NotificationWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (QWindow *w = qobject_cast<QWindow*>(object)) {
            if (!w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
                QQuickView::setVisible(false);
            }
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            QQuickView::setVisible(false);
        }
    } else if (event->type() == QEvent::Show) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        HistoryModel::self()->updateTime();
    } else if (event->type() == QEvent::Hide) {
        setMouseGrabEnabled(false);
        setKeyboardGrabEnabled(false);
    }

    return QObject::eventFilter(object, event);
}
