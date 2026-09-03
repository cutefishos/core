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

#include <QGuiApplication>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include <QDebug>

#include <KIdleTime>

#include <utility>

#include <KWayland/Client/connection_thread.h>
#include <KWayland/Client/dpms.h>
#include <KWayland/Client/output.h>
#include <KWayland/Client/registry.h>

namespace
{
constexpr auto s_screenSaverService = "org.freedesktop.ScreenSaver";
constexpr auto s_screenSaverPath = "/ScreenSaver";
constexpr auto s_screenSaverInterface = "org.freedesktop.ScreenSaver";
constexpr auto s_kdeScreenSaverService = "org.kde.screensaver";
constexpr auto s_kdeScreenSaverInterface = "org.kde.screensaver";
constexpr auto s_logindService = "org.freedesktop.login1";
constexpr auto s_logindPath = "/org/freedesktop/login1";
constexpr auto s_logindInterface = "org.freedesktop.login1.Manager";

constexpr int s_brightnessDimTimeoutNumerator = 1;
constexpr int s_brightnessDimTimeoutDenominator = 2;
constexpr int s_brightnessDimTimeoutFinalNumerator = 3;
constexpr int s_brightnessDimTimeoutFinalDenominator = 4;
}

DimDisplayAction::DimDisplayAction(QObject *parent)
    : Action(parent)
    , m_iface("com.cutefish.Services",
              "/com/cutefish/Services/Brightness",
              "com.cutefish.Services.Brightness", QDBusConnection::sessionBus())
{
    QDBusConnection::sessionBus().connect(QString::fromLatin1(s_screenSaverService),
                                          QString::fromLatin1(s_screenSaverPath),
                                          QString::fromLatin1(s_screenSaverInterface),
                                          QStringLiteral("ActiveChanged"),
                                          this,
                                          SLOT(onScreenSaverActiveChanged(bool)));
    QDBusConnection::systemBus().connect(QString::fromLatin1(s_logindService),
                                         QString::fromLatin1(s_logindPath),
                                         QStringLiteral("org.freedesktop.DBus.Properties"),
                                         QStringLiteral("PropertiesChanged"),
                                         this,
                                         SLOT(onLogindPropertiesChanged(QString,QVariantMap,QStringList)));
    m_suspendTimer.setSingleShot(true);
    connect(&m_suspendTimer, &QTimer::timeout,
            this, &DimDisplayAction::onSuspendTimerTimeout);
    setupWaylandDpms();
}

void DimDisplayAction::setLidPresent(bool present)
{
    if (m_lidPresent == present)
        return;

    m_lidPresent = present;
    if (m_lidPresent) {
        inhibitSystemdLidSwitch();
    } else {
        m_lidSwitchInhibitor = QDBusUnixFileDescriptor();
    }
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

    m_dimmed = false;
    m_suspendRequested = false;
    if (!m_lidClosed) {
        m_suspendTimer.stop();
        m_suspendDue = false;
        m_displayOffTimer.invalidate();
    }

    // Wait one event loop turn for the screen to wake up before restoring it.
    QTimer::singleShot(0, this, [this]() {
        if (!m_lidClosed)
            restoreDisplay();
    });
}

void DimDisplayAction::onIdleTimeout(int msec)
{
    // KIdleTime may already have queued a timeout notification when the
    // policy is changed. Never trigger the idle action if that stale
    // notification is delivered after screen-off has been disabled.
    if (m_dimOnIdleTime < 0)
        return;

    int sec = msec / 1000;

    if (sec == m_dimOnIdleTime && !m_displayOff) {
        // KScreenLocker owns the automatic lock and its inhibitor handling.
        // Wait for ActiveChanged before powering off the display so locking
        // is established before DPMS is disabled.
        m_idleLockPending = true;
        return;
    }

    if (!hasSupportedWaylandDpms() && sec == (m_dimOnIdleTime * s_brightnessDimTimeoutFinalNumerator / s_brightnessDimTimeoutFinalDenominator)) {
        if (m_oldScreenBrightness > 0) {
            const int newBrightness = qRound(m_oldScreenBrightness / 8.0);
            m_iface.asyncCall("setValue", QVariant::fromValue(newBrightness));
        }
        m_dimmed = true;
    } else if (!hasSupportedWaylandDpms() && sec == (m_dimOnIdleTime * s_brightnessDimTimeoutNumerator / s_brightnessDimTimeoutDenominator)) {
        m_oldScreenBrightness = m_iface.property("brightness").toInt();

        if (m_oldScreenBrightness > 0) {
            const int newBrightness = qRound(m_oldScreenBrightness / 2.0);
            m_iface.asyncCall("setValue", QVariant::fromValue(newBrightness));
        }
        m_dimmed = true;
    }
}

void DimDisplayAction::setTimeout(int timeout)
{
    setPolicy(timeout, m_suspendDelay);
}

void DimDisplayAction::setPolicy(int timeout, int suspendDelay)
{
    unregisterIdleTimeout();

    m_dimOnIdleTime = timeout;
    m_suspendDelay = qMax(0, suspendDelay);
    m_idleLockPending = false;
    m_suspendTimer.stop();
    if (!m_lidClosed)
        m_suspendDue = false;

    configureScreenLocker(timeout);

    if (timeout < 0) {
        m_displayOffTimer.invalidate();
        return;
    }

    const int halfTimeout = timeout * s_brightnessDimTimeoutNumerator / s_brightnessDimTimeoutDenominator;
    const int finalDimTimeout = timeout * s_brightnessDimTimeoutFinalNumerator / s_brightnessDimTimeoutFinalDenominator;

    if (halfTimeout > 0)
        registerIdleTimeout(halfTimeout);
    if (finalDimTimeout > halfTimeout && finalDimTimeout < timeout)
        registerIdleTimeout(finalDimTimeout);
    registerIdleTimeout(timeout);

    if (m_displayOff && !m_lidDisplayOff && m_displayOffTimer.isValid() && m_suspendDelay > 0) {
        const qint64 remaining = qint64(m_suspendDelay) * 1000 - m_displayOffTimer.elapsed();
        if (remaining <= 0) {
            m_suspendDue = true;
            trySuspend();
        } else {
            m_suspendTimer.start(int(remaining));
        }
    }
}

void DimDisplayAction::configureScreenLocker(int timeout)
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation)
                       + QStringLiteral("/kscreenlockerrc"),
                       QSettings::IniFormat);
    settings.beginGroup(QStringLiteral("Daemon"));
    settings.setValue(QStringLiteral("Autolock"), timeout >= 0);
    if (timeout >= 0)
        settings.setValue(QStringLiteral("Timeout"), qMax(1, timeout / 60));
    settings.endGroup();
    settings.sync();

    QDBusInterface screenSaver(QString::fromLatin1(s_kdeScreenSaverService),
                               QString::fromLatin1(s_screenSaverPath),
                               QString::fromLatin1(s_kdeScreenSaverInterface),
                               QDBusConnection::sessionBus());
    if (screenSaver.isValid())
        screenSaver.asyncCall(QStringLiteral("configure"));
}

void DimDisplayAction::onScreenSaverActiveChanged(bool active)
{
    if (!active) {
        m_idleLockPending = false;
        m_dimmed = false;

        if (!m_lidClosed) {
            m_suspendTimer.stop();
            m_suspendDue = false;
            m_displayOffTimer.invalidate();

            if (m_displayOff)
                restoreDisplay();
        }
        return;
    }

    if (m_displayOff || m_dimOnIdleTime < 0)
        return;

    const int idleTime = KIdleTime::instance()->idleTime();
    if (!m_idleLockPending && idleTime < m_dimOnIdleTime * 1000)
        return;

    m_idleLockPending = false;
    turnDisplayOff();
    m_dimmed = true;
    startSuspendCountdown();

    trySuspend();
}

void DimDisplayAction::onLogindPropertiesChanged(const QString &interfaceName,
                                                 const QVariantMap &changedProperties,
                                                 const QStringList &invalidatedProperties)
{
    if (interfaceName != QString::fromLatin1(s_logindInterface))
        return;

    if (!changedProperties.contains(QStringLiteral("BlockInhibited"))
        && !changedProperties.contains(QStringLiteral("BlockWeakInhibited"))
        && !changedProperties.contains(QStringLiteral("DelayInhibited"))
        && !invalidatedProperties.contains(QStringLiteral("BlockInhibited"))
        && !invalidatedProperties.contains(QStringLiteral("BlockWeakInhibited"))
        && !invalidatedProperties.contains(QStringLiteral("DelayInhibited"))) {
        return;
    }

    trySuspend();
}

void DimDisplayAction::onSuspendTimerTimeout()
{
    if (!m_displayOff)
        return;

    m_suspendDue = true;
    trySuspend();
}

bool DimDisplayAction::lockSession()
{
    QDBusInterface screenSaver(QStringLiteral("org.freedesktop.ScreenSaver"),
                               QStringLiteral("/ScreenSaver"),
                               QStringLiteral("org.freedesktop.ScreenSaver"),
                               QDBusConnection::sessionBus());
    if (!screenSaver.isValid())
        return false;

    // KScreenLocker uses a delayed D-Bus reply until the lock is actually
    // established. Wait for that reply before powering the display off.
    const QDBusMessage reply = screenSaver.call(QStringLiteral("Lock"));
    if (reply.type() != QDBusMessage::ReplyMessage) {
        qWarning() << "Unable to lock the session:" << reply.errorMessage();
        return false;
    }

    return true;
}

bool DimDisplayAction::suspendSession()
{
    QDBusInterface session(QStringLiteral("com.cutefish.Session"),
                           QStringLiteral("/Session"),
                           QStringLiteral("com.cutefish.Session"),
                           QDBusConnection::sessionBus());
    if (!session.isValid())
        return false;

    const QDBusMessage reply = session.call(QStringLiteral("suspend"));
    return reply.type() == QDBusMessage::ReplyMessage
        && !reply.arguments().isEmpty()
        && reply.arguments().constFirst().toBool();
}

void DimDisplayAction::trySuspend()
{
    if (!m_suspendDue || !m_displayOff || m_suspendRequested)
        return;

    if (suspendSession()) {
        m_suspendRequested = true;
        m_suspendDue = false;
    }
}

void DimDisplayAction::startSuspendCountdown()
{
    m_suspendTimer.stop();
    m_suspendDue = false;

    if (m_lidClosed)
        return;

    m_displayOffTimer.restart();
    if (m_suspendDelay > 0)
        m_suspendTimer.start(m_suspendDelay * 1000);
}

void DimDisplayAction::turnDisplayOff()
{
    if (m_displayOff)
        return;

    m_displayPoweredOff = setDisplayPower(false);

    if (!m_displayPoweredOff) {
        if (m_oldScreenBrightness < 0)
            m_oldScreenBrightness = m_iface.property("brightness").toInt();

        if (m_oldScreenBrightness > 0)
            m_iface.asyncCall("setValue", QVariant::fromValue(0));
    }

    m_displayOff = true;
}

void DimDisplayAction::restoreDisplay()
{
    if (m_displayPoweredOff) {
        setDisplayPower(true);
        m_displayPoweredOff = false;
    }

    if (m_oldScreenBrightness >= 0)
        m_iface.asyncCall("setValue", QVariant::fromValue(m_oldScreenBrightness));

    m_oldScreenBrightness = -1;
    m_displayOff = false;
    m_displayOffTimer.invalidate();
}

void DimDisplayAction::handleLidClosed()
{
    m_lidClosed = true;

    if (!m_lidPresent)
        return;

    if (!m_lidDisplayOff) {
        if (!m_displayOff) {
            if (!lockSession())
                return;

            turnDisplayOff();
            m_dimmed = true;
        }

        m_lidDisplayOff = true;
    }

    m_suspendTimer.stop();
    m_displayOffTimer.invalidate();
    if (!m_suspendRequested) {
        m_suspendDue = true;
        trySuspend();
    }
}

void DimDisplayAction::handleLidOpened()
{
    m_lidClosed = false;
    m_suspendRequested = false;
    m_suspendDue = false;
    m_suspendTimer.stop();
    m_displayOffTimer.invalidate();

    if (!m_lidDisplayOff)
        return;

    m_lidDisplayOff = false;
    restoreDisplay();
}

void DimDisplayAction::inhibitSystemdLidSwitch()
{
    if (!m_lidPresent || m_lidSwitchInhibitor.isValid()) {
        return;
    }

    QDBusInterface logind(QString::fromLatin1(s_logindService),
                          QString::fromLatin1(s_logindPath),
                          QString::fromLatin1(s_logindInterface),
                          QDBusConnection::systemBus());
    if (!logind.isValid())
        return;

    const QDBusMessage reply = logind.call(QStringLiteral("Inhibit"),
                                           QStringLiteral("handle-lid-switch"),
                                           QStringLiteral("Cutefish Settings"),
                                           QStringLiteral("Cutefish manages lid power policy"),
                                           QStringLiteral("block"));
    if (reply.type() != QDBusMessage::ReplyMessage || reply.arguments().isEmpty()) {
        qWarning() << "Unable to inhibit systemd lid handling:" << reply.errorMessage();
        return;
    }

    const auto inhibitor = reply.arguments().constFirst().value<QDBusUnixFileDescriptor>();
    if (!inhibitor.isValid()) {
        qWarning() << "systemd returned an invalid lid inhibitor";
        return;
    }

    m_lidSwitchInhibitor = inhibitor;
}
