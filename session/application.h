/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QApplication>

#include "processmanager.h"
#include "networkproxymanager.h"
#include "powermanager/power.h"

class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);

    bool wayland() const;

public slots:
    void logout() {
        m_processManager->logout();
    }

    void reboot() {
        m_power.reboot();
        QCoreApplication::exit(0);
    }

    void powerOff() {
        m_power.shutdown();
        QCoreApplication::exit(0);
    }

    void suspend() {
        m_power.suspend();
    }

    void startDesktopProcess() {
        m_processManager->startDesktopProcess();
    }

    void updateNetworkProxy() {
        m_networkProxyManager->update();
    }

    void launch(const QString &exec, const QStringList &args);
    void launch(const QString &exec, const QString &workingDir, const QStringList &args);

private:
    void initEnvironments();
    void initLanguage();
    void initScreenScaleFactors();
    void initXResource();
    void initKWinConfig();
    bool syncDBusEnvironment();
    void importSystemdEnvrionment();
    void createConfigDirectory();
    void updateUserDirs();
    int runSync(const QString &program, const QStringList &args, const QStringList &env = {});

private:
    ProcessManager *m_processManager;
    NetworkProxyManager *m_networkProxyManager;
    Power m_power;

    bool m_wayland;
};

#endif // APPLICATION_H
