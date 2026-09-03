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

#include "processmanager.h"
#include "application.h"

#include <QCoreApplication>
#include <QStandardPaths>
#include <QSettings>
#include <QDebug>
#include <QProcessEnvironment>
#include <QTimer>
#include <QDir>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QDBusServiceWatcher>

ProcessManager::ProcessManager(Application *app, QObject *parent)
    : QObject(parent)
    , m_app(app)
{
}

ProcessManager::~ProcessManager()
{
    QMapIterator<QString, QProcess *> i(m_systemProcess);
    while (i.hasNext()) {
        i.next();
        QProcess *p = i.value();
        delete p;
        m_systemProcess[i.key()] = nullptr;
    }
}

void ProcessManager::start()
{
    if (m_kwinReady)
        return;

    const QString serviceName = QStringLiteral("org.kde.KWinWrapper");
    m_kwinWatcher = new QDBusServiceWatcher(serviceName,
                                             QDBusConnection::sessionBus(),
                                             QDBusServiceWatcher::WatchForRegistration,
                                             this);
    connect(m_kwinWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, [this](const QString &) {
        startAfterKWinReady();
    });

    if (QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface()) {
        const QDBusReply<bool> reply = interface->isServiceRegistered(serviceName);
        if (reply.isValid() && reply.value())
            startAfterKWinReady();
    }
}

void ProcessManager::startAfterKWinReady()
{
    if (m_kwinReady)
        return;

    m_kwinReady = true;
    if (m_kwinWatcher) {
        m_kwinWatcher->deleteLater();
        m_kwinWatcher = nullptr;
    }

    // Start the desktop after the services daemon has registered its D-Bus
    // service so its theme and input backends are available to the shell.
    const QString serviceName = QStringLiteral("com.cutefish.Services");
    const auto servicesReady = [this]() {
        if (m_servicesWatcher) {
            m_servicesWatcher->deleteLater();
            m_servicesWatcher = nullptr;
        }
        startDesktopProcess();
    };

    m_servicesWatcher = new QDBusServiceWatcher(serviceName,
                                                QDBusConnection::sessionBus(),
                                                QDBusServiceWatcher::WatchForRegistration,
                                                this);
    connect(m_servicesWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, [servicesReady](const QString &) {
        servicesReady();
    });

    startDaemonProcess();

    if (QDBusConnectionInterface *interface = QDBusConnection::sessionBus().interface()) {
        const QDBusReply<bool> reply = interface->isServiceRegistered(serviceName);
        if (reply.isValid() && reply.value())
            servicesReady();
    }
}

void ProcessManager::logout()
{
    // Close what we started ourselves, the window manager last since
    // everything else is drawn on top of it.
    stopProcesses(m_autoStartProcess);
    stopProcesses(m_systemProcess);

    // KWin is started with --exit-with-session, so returning from this
    // process also ends the compositor session and returns to the greeter.

    QCoreApplication::exit(0);
}

void ProcessManager::stopProcesses(QMap<QString, QProcess *> &processes)
{
    QMapIterator<QString, QProcess *> i(processes);
    while (i.hasNext()) {
        i.next();
        QProcess *p = i.value();

        if (!p || p->state() == QProcess::NotRunning)
            continue;

        p->terminate();

        if (!p->waitForFinished(2000))
            p->kill();
    }
}

void ProcessManager::startDesktopProcess()
{
    if (!m_kwinReady) {
        qWarning() << "Ignoring desktop startup before KWin is ready";
        return;
    }

    if (m_desktopStarted)
        return;

    m_desktopStarted = true;

    QList<QPair<QString, QStringList>> list;
    // Desktop components
    // The status bar, dock, launcher, desktop and notifications are one process now.
    list << qMakePair(QString("cutefish-shell"), QStringList());
    list << qMakePair(QString("cutefish-clipboard"), QStringList());

    // For CutefishOS.
    if (QFile("/usr/bin/cutefish-welcome").exists() &&
            !QFile("/run/live/medium/live/filesystem.squashfs").exists()) {
        QSettings settings("cutefishos", "login");

        if (!settings.value("Finished", false).toBool()) {
            list << qMakePair(QString("/usr/bin/cutefish-welcome"), QStringList());
        } else {
            list << qMakePair(QString("/usr/bin/cutefish-welcome"), QStringList() << "-d");
        }
    }

    for (QPair<QString, QStringList> pair : list) {
        QProcess *process = new QProcess;
        process->setProcessChannelMode(QProcess::ForwardedChannels);
        process->setProgram(pair.first);
        process->setArguments(pair.second);
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("wayland"));
        process->setProcessEnvironment(environment);
        process->start();
        process->waitForStarted();

        qDebug() << "Load DE components: " << pair.first << pair.second;

        // Add to map
        if (process->state() != QProcess::NotRunning) {
            m_autoStartProcess.insert(pair.first, process);
        } else {
            qWarning() << "Failed to start desktop process:" << pair.first
                       << process->errorString();
            process->deleteLater();
        }
    }

    // Auto start
    QTimer::singleShot(100, this, &ProcessManager::loadAutoStartProcess);
}

void ProcessManager::startDaemonProcess()
{
    QList<QPair<QString, QStringList>> list;
    // This daemon provides the services used by the desktop components.
    list << qMakePair(QString("cutefish-services"), QStringList());

    for (QPair<QString, QStringList> pair : list) {
        QProcess *process = new QProcess;
        process->setProcessChannelMode(QProcess::ForwardedChannels);
        process->setProgram(pair.first);
        process->setArguments(pair.second);
        process->start();
        process->waitForStarted();

        // Add to map
        // exitCode() is not a valid way to test a process that is still
        // running. The old check discarded long-lived daemons immediately
        // after starting them.
        if (process->state() != QProcess::NotRunning) {
            m_autoStartProcess.insert(pair.first, process);
        } else {
            qWarning() << "Failed to start daemon:" << pair.first
                       << process->errorString();
            process->deleteLater();
        }
    }
}

void ProcessManager::loadAutoStartProcess()
{
    QStringList execList;
    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::GenericConfigLocation,
                                                       QStringLiteral("autostart"),
                                                       QStandardPaths::LocateDirectory);
    for (const QString &dir : dirs) {
        const QDir d(dir);
        const QStringList fileNames = d.entryList(QStringList() << QStringLiteral("*.desktop"));
        for (const QString &file : fileNames) {
            QSettings desktop(d.absoluteFilePath(file), QSettings::IniFormat);
            desktop.beginGroup("Desktop Entry");

            if (desktop.contains("OnlyShowIn"))
                continue;

            const QString execValue = desktop.value("Exec").toString();

            if (execValue.contains("cutefish-services"))
                continue;

            if (!execValue.isEmpty()) {
                execList << execValue;
            }
        }
    }

    for (const QString &exec : execList) {
        QProcess *process = new QProcess;
        process->setProgram(exec);
        QProcessEnvironment environment = QProcessEnvironment::systemEnvironment();
        environment.insert(QStringLiteral("QT_QPA_PLATFORM"), QStringLiteral("wayland"));
        process->setProcessEnvironment(environment);
        process->start();
        process->waitForStarted();

        if (process->exitCode() == 0) {
            m_autoStartProcess.insert(exec, process);
        } else {
            process->deleteLater();
        }
    }
}
