/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#include "application.h"
#include "powermanageradaptor.h"

#include <QDBusConnection>
#include <QDebug>

Application::Application(QObject *parent)
    : QObject(parent)
    , m_lidWatcher(new LidWatcher)
    , m_cpuManagement(new CPUManagement)
    , m_settings("cutefishos", "power")
    , m_dimDisplayAction(new DimDisplayAction)
{
    m_closeScreenTimeout = m_settings.value("CloseScreenTimeout", 600).toInt();
    m_sleepWhenClosedScreen = m_settings.value("SleepWhenClosedScreen", false).toBool();
    m_lockWhenClosedScreen = m_settings.value("LockWhenClosedScreen", true).toBool();

    // Live
    if (QFile("/run/live/medium/live/filesystem.squashfs").exists()) {
        m_closeScreenTimeout = -1;
    }

    m_dimDisplayAction->setTimeout(m_closeScreenTimeout);
    m_dimDisplayAction->setSleep(m_sleepWhenClosedScreen);
    m_dimDisplayAction->setLock(m_lockWhenClosedScreen);

    QDBusConnection::sessionBus().registerService(QStringLiteral("com.cutefish.PowerManager"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/PowerManager"), this);
    new PowerManagerAdaptor(this);
}

void Application::setDimDisplayTimeout(int timeout)
{
    m_closeScreenTimeout = timeout;
    m_dimDisplayAction->setTimeout(timeout);

    m_settings.setValue("CloseScreenTimeout", timeout);
}

void Application::setSleepWhenClosedScreen(bool enabled)
{
    m_sleepWhenClosedScreen = enabled;
    m_dimDisplayAction->setSleep(enabled);

    m_settings.setValue("SleepWhenClosedScreen", enabled);
}

void Application::setLockWhenClosedScreen(bool lock)
{
    m_lockWhenClosedScreen = lock;
    m_dimDisplayAction->setLock(lock);

    m_settings.setValue("LockWhenClosedScreen", lock);
}
