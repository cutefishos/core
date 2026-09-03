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

#ifndef DIMDISPLAYACTION_H
#define DIMDISPLAYACTION_H

#include "action.h"
#include <QElapsedTimer>
#include <QList>
#include <QDBusInterface>
#include <QDBusUnixFileDescriptor>
#include <QPointer>
#include <QTimer>

namespace KWayland
{
namespace Client
{
class ConnectionThread;
class Dpms;
class DpmsManager;
class Output;
class Registry;
}
}

class DimDisplayAction : public Action
{
    Q_OBJECT

public:
    explicit DimDisplayAction(QObject *parent = nullptr);

    void onWakeupFromIdle() override;
    void onIdleTimeout(int msec) override;
    void setTimeout(int timeout) override;
    void setPolicy(int timeout, int suspendDelay);
    void setLidPresent(bool present);
    void handleLidClosed();
    void handleLidOpened();

private Q_SLOTS:
    void onScreenSaverActiveChanged(bool active);
    void onLogindPropertiesChanged(const QString &interfaceName,
                                   const QVariantMap &changedProperties,
                                   const QStringList &invalidatedProperties);
    void onSuspendTimerTimeout();

private:
    bool lockSession();
    bool suspendSession();
    void trySuspend();
    void turnDisplayOff();
    void restoreDisplay();
    void configureScreenLocker(int timeout);
    void startSuspendCountdown();
    void inhibitSystemdLidSwitch();
    void setupWaylandDpms();
    void setupWaylandOutput(quint32 name, quint32 version);
    void setupWaylandDpmsManager(quint32 name, quint32 version);
    void addWaylandDpms(KWayland::Client::Output *output);
    bool hasSupportedWaylandDpms() const;
    bool setDisplayPower(bool on);

    QDBusInterface m_iface;
    int m_dimOnIdleTime = 0;
    int m_suspendDelay = 900;
    int m_oldScreenBrightness = -1;
    bool m_dimmed = false;
    bool m_displayPoweredOff = false;
    bool m_displayOff = false;
    bool m_suspendRequested = false;
    bool m_idleLockPending = false;
    bool m_suspendDue = false;
    bool m_lidClosed = false;
    bool m_lidPresent = false;
    bool m_lidDisplayOff = false;

    QDBusUnixFileDescriptor m_lidSwitchInhibitor;

    KWayland::Client::ConnectionThread *m_waylandConnection = nullptr;
    KWayland::Client::Registry *m_waylandRegistry = nullptr;
    KWayland::Client::DpmsManager *m_waylandDpmsManager = nullptr;
    QList<QPointer<KWayland::Client::Output>> m_waylandOutputs;
    QList<QPointer<KWayland::Client::Dpms>> m_waylandDpmsOutputs;

    QTimer m_suspendTimer;
    QElapsedTimer m_displayOffTimer;
};

#endif // DIMDISPLAYACTION_H
