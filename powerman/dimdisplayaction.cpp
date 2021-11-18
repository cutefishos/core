/***************************************************************************
 *   Copyright (C) 2021 by Reion Wong <reion@cutefishos.com>               *
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "dimdisplayaction.h"
#include <QSettings>
#include <QTimer>
#include <QDBusPendingCall>
#include <QX11Info>
#include <QProcess>
#include <QDebug>

#include <X11/Xlib.h>
#include <xcb/dpms.h>

DimDisplayAction::DimDisplayAction(QObject *parent)
    : Action(parent)
    , m_iface("com.cutefish.Settings",
              "/Brightness",
              "com.cutefish.Brightness", QDBusConnection::sessionBus())
{
    if (QX11Info::isPlatformX11()) {
        // Disable a default timeout, if any
        xcb_dpms_set_timeouts(QX11Info::connection(), 0, 0, 0);

        XSetScreenSaver(QX11Info::display(), 0, 0, 0, 0);
    }
}

void DimDisplayAction::onWakeupFromIdle()
{   
    if (!m_dimmed) {
        return;
    }

    if (m_oldScreenBrightness < 0)
        m_oldScreenBrightness = 1;

    // An active inhibition may not let us restore the brightness.
    // We should wait a bit screen to wake-up from sleep
    QTimer::singleShot(0, this, [this]() {
        m_iface.asyncCall("setValue", QVariant::fromValue(m_oldScreenBrightness));
    });

    m_dimmed = false;
}

void DimDisplayAction::onIdleTimeout(int msec)
{
    int sec = msec / 1000;

    if (m_iface.property("brightness").toInt() == 0)
        return;

    if (sec == m_dimOnIdleTime) {
        m_iface.asyncCall("setValue", QVariant::fromValue(0));

        // Sleep
        if (m_sleep) {
            QDBusInterface iface("com.cutefish.Session",
                                 "/Session",
                                 "com.cutefish.Session", QDBusConnection::sessionBus());

            if (iface.isValid()) {
                iface.call("suspend");
            }
        }

        if (m_lock) {
            QProcess::startDetached("cutefish-screenlocker", QStringList());
        }

    } else if (sec == (m_dimOnIdleTime * 3 / 4)) {
        const int newBrightness = qRound(m_oldScreenBrightness / 8.0);
        m_iface.asyncCall("setValue", QVariant::fromValue(newBrightness));
    } else if (sec == (m_dimOnIdleTime * 1 / 2)) {
        m_oldScreenBrightness = m_iface.property("brightness").toInt();

        const int newBrightness = qRound(m_oldScreenBrightness / 2.0);

        m_iface.asyncCall("setValue", QVariant::fromValue(newBrightness));
    }

    m_dimmed = true;
}

void DimDisplayAction::setTimeout(int timeout)
{
    unregisterIdleTimeout();

    if (timeout < 0) {
        m_dimOnIdleTime = timeout;
        return;
    }

    m_dimOnIdleTime = timeout;
    registerIdleTimeout(m_dimOnIdleTime * 3 / 4);
    registerIdleTimeout(m_dimOnIdleTime / 2);
    registerIdleTimeout(m_dimOnIdleTime);
}

void DimDisplayAction::setSleep(bool sleep)
{
    m_sleep = sleep;
}

void DimDisplayAction::setLock(bool lock)
{
    m_lock = lock;
}
