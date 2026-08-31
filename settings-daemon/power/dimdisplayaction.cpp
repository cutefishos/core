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
#include "screenlocker_interface.h"

#include <QGuiApplication>
#include <QSettings>
#include <QTimer>
#include <QDBusPendingCall>
#include <QDebug>

#include <utility>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/dpms.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/registry.h>

DimDisplayAction::DimDisplayAction(QObject *parent)
    : Action(parent)
    , m_iface("com.cutefish.Settings",
              "/Brightness",
              "com.cutefish.Brightness", QDBusConnection::sessionBus())
{
    setupWaylandDpms();
}

void DimDisplayAction::setupWaylandDpms()
{
    if (!QGuiApplication::platformName().startsWith(QStringLiteral("wayland")))
        return;

    m_waylandConnection = KWayland::Client::ConnectionThread::fromApplication(this);
    if (!m_waylandConnection)
        return;

    m_waylandRegistry = new KWayland::Client::Registry(this);
    m_waylandRegistry->create(m_waylandConnection);

    connect(m_waylandRegistry, &KWayland::Client::Registry::outputAnnounced,
            this, &DimDisplayAction::setupWaylandOutput);
    connect(m_waylandRegistry, &KWayland::Client::Registry::dpmsAnnounced,
            this, &DimDisplayAction::setupWaylandDpmsManager);

    m_waylandRegistry->setup();
    m_waylandConnection->roundtrip();
    // DPMS support is announced after the output objects are created.
    m_waylandConnection->roundtrip();
}

void DimDisplayAction::setupWaylandOutput(quint32 name, quint32 version)
{
    auto *output = m_waylandRegistry->createOutput(name, version, this);
    if (!output || !output->isValid())
        return;

    m_waylandOutputs.append(output);
    addWaylandDpms(output);
}

void DimDisplayAction::setupWaylandDpmsManager(quint32 name, quint32 version)
{
    if (m_waylandDpmsManager)
        return;

    m_waylandDpmsManager = m_waylandRegistry->createDpmsManager(name, version, this);
    if (!m_waylandDpmsManager || !m_waylandDpmsManager->isValid()) {
        m_waylandDpmsManager = nullptr;
        return;
    }

    for (const auto &output : std::as_const(m_waylandOutputs)) {
        if (output)
            addWaylandDpms(output);
    }
}

void DimDisplayAction::addWaylandDpms(KWayland::Client::Output *output)
{
    if (!m_waylandDpmsManager || !output)
        return;

    auto *dpms = m_waylandDpmsManager->getDpms(output, this);
    if (dpms && dpms->isValid())
        m_waylandDpmsOutputs.append(dpms);
}

bool DimDisplayAction::hasSupportedWaylandDpms() const
{
    for (const auto &dpms : std::as_const(m_waylandDpmsOutputs)) {
        if (dpms && dpms->isSupported())
            return true;
    }

    return false;
}

bool DimDisplayAction::setDisplayPower(bool on)
{
    bool requested = false;
    const auto mode = on ? KWayland::Client::Dpms::Mode::On
                         : KWayland::Client::Dpms::Mode::Off;

    for (const auto &dpms : std::as_const(m_waylandDpmsOutputs)) {
        if (!dpms || !dpms->isSupported())
            continue;

        dpms->requestMode(mode);
        requested = true;
    }

    return requested;
}

void DimDisplayAction::onWakeupFromIdle()
{   
    if (!m_dimmed) {
        return;
    }

    // An active inhibition may not let us restore the brightness.
    // We should wait a bit screen to wake-up from sleep
    QTimer::singleShot(0, this, [this]() {
        if (m_displayPoweredOff) {
            setDisplayPower(true);
            m_displayPoweredOff = false;
        }

        if (m_oldScreenBrightness >= 0)
            m_iface.asyncCall("setValue", QVariant::fromValue(m_oldScreenBrightness));
    });

    m_oldScreenBrightness = -1;
    m_dimmed = false;
}

void DimDisplayAction::onIdleTimeout(int msec)
{
    int sec = msec / 1000;

    if (sec == m_dimOnIdleTime) {
        m_displayPoweredOff = setDisplayPower(false);

        if (!m_displayPoweredOff && m_iface.property("brightness").toInt() > 0)
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
            OrgFreedesktopScreenSaverInterface screenSaver(
                QStringLiteral("org.freedesktop.ScreenSaver"),
                QStringLiteral("/ScreenSaver"),
                QDBusConnection::sessionBus());
            screenSaver.Lock();
        }

    } else if (!hasSupportedWaylandDpms() && sec == (m_dimOnIdleTime * 3 / 4)) {
        if (m_oldScreenBrightness > 0) {
            const int newBrightness = qRound(m_oldScreenBrightness / 8.0);
            m_iface.asyncCall("setValue", QVariant::fromValue(newBrightness));
        }
    } else if (!hasSupportedWaylandDpms() && sec == (m_dimOnIdleTime * 1 / 2)) {
        m_oldScreenBrightness = m_iface.property("brightness").toInt();

        if (m_oldScreenBrightness > 0) {
            const int newBrightness = qRound(m_oldScreenBrightness / 2.0);
            m_iface.asyncCall("setValue", QVariant::fromValue(newBrightness));
        }
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
