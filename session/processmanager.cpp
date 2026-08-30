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
#include <QFileInfoList>
#include <QFileInfo>
#include <QSettings>
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QDir>
#include <QtGui/qguiapplication_platform.h>

#include <QDBusInterface>
#include <QDBusPendingCall>

#include <KWindowSystem>
#include <netwm.h>

ProcessManager::ProcessManager(Application *app, QObject *parent)
    : QObject(parent)
    , m_app(app)
    , m_wmStarted(false)
    , m_waitLoop(nullptr)
{
    qApp->installNativeEventFilter(this);
}

ProcessManager::~ProcessManager()
{
    qApp->removeNativeEventFilter(this);

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
    startWindowManager();
    startDaemonProcess();
}

void ProcessManager::logout()
{
    QDBusInterface kwinIface("org.kde.KWin",
                             "/Session",
                             "org.kde.KWin.Session",
                             QDBusConnection::sessionBus());

    if (kwinIface.isValid()) {
        kwinIface.call("aboutToSaveSession", "cutefish");
        kwinIface.call("setState", uint(1)); // Quitting
    }

    // Close what we started ourselves, the window manager last since
    // everything else is drawn on top of it.
    stopProcesses(m_autoStartProcess);
    stopProcesses(m_systemProcess);

    // Ending the session is up to whoever started us: the display manager
    // closes the login session and returns to the greeter once we exit.
    // Calling Terminate on the logind session here would kill the display
    // manager helper along with us, before it can do any of that, and leave
    // the user staring at a black screen instead of the greeter.
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

void ProcessManager::startWindowManager()
{
    QProcess *wmProcess = new QProcess;
    m_systemProcess.insert("kwin", wmProcess);

    wmProcess->start(m_app->wayland() ? "kwin_wayland" : "kwin_x11", QStringList());

    if (!m_app->wayland()) {
        QEventLoop waitLoop;
        m_waitLoop = &waitLoop;
        // add a timeout to avoid infinite blocking if a WM fail to execute.
        QTimer::singleShot(30 * 1000, &waitLoop, SLOT(quit()));
        waitLoop.exec();
        m_waitLoop = nullptr;
    }
}

void ProcessManager::startDesktopProcess()
{
    // When the cutefish-settings-daemon theme module is loaded, start the desktop.
    // In the way, there will be no problem that desktop and launcher can't get wallpaper.

    QList<QPair<QString, QStringList>> list;
    // Desktop components
    list << qMakePair(QString("cutefish-notificationd"), QStringList());
    // The status bar, dock, launcher and desktop are one process now.
    list << qMakePair(QString("cutefish-shell"), QStringList());
    list << qMakePair(QString("cutefish-powerman"), QStringList());
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
        process->start();
        process->waitForStarted();

        qDebug() << "Load DE components: " << pair.first << pair.second;

        // Add to map
        if (process->exitCode() == 0) {
            m_autoStartProcess.insert(pair.first, process);
        } else {
            process->deleteLater();
        }
    }

    // Auto start
    QTimer::singleShot(100, this, &ProcessManager::loadAutoStartProcess);
}

void ProcessManager::startDaemonProcess()
{
    QList<QPair<QString, QStringList>> list;
    list << qMakePair(QString("cutefish-settings-daemon"), QStringList());
    list << qMakePair(QString("cutefish-xembedsniproxy"), QStringList());
    list << qMakePair(QString("cutefish-gmenuproxy"), QStringList());
    list << qMakePair(QString("chotkeys"), QStringList());

    for (QPair<QString, QStringList> pair : list) {
        QProcess *process = new QProcess;
        process->setProcessChannelMode(QProcess::ForwardedChannels);
        process->setProgram(pair.first);
        process->setArguments(pair.second);
        process->start();
        process->waitForStarted();

        // Add to map
        if (process->exitCode() == 0) {
            m_autoStartProcess.insert(pair.first, process);
        } else {
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

            // 避免冲突
            if (execValue.contains("gmenudbusmenuproxy"))
                continue;

            if (!execValue.isEmpty()) {
                execList << execValue;
            }
        }
    }

    for (const QString &exec : execList) {
        QProcess *process = new QProcess;
        process->setProgram(exec);
        process->start();
        process->waitForStarted();

        if (process->exitCode() == 0) {
            m_autoStartProcess.insert(exec, process);
        } else {
            process->deleteLater();
        }
    }
}

bool ProcessManager::nativeEventFilter(const QByteArray &eventType, void *message, qintptr *result)
{
    if (eventType != "xcb_generic_event_t") // We only want to handle XCB events
        return false;

    // ref: lxqt session
    if (!m_wmStarted && m_waitLoop) {
        // all window managers must set their name according to the spec
        if (!QString::fromUtf8(NETRootInfo(qGuiApp->nativeInterface<QNativeInterface::QX11Application>()->connection(), NET::SupportingWMCheck).wmName()).isEmpty()) {
            qDebug() << "Window manager started";
            m_wmStarted = true;
            if (m_waitLoop && m_waitLoop->isRunning())
                m_waitLoop->exit();

            qApp->removeNativeEventFilter(this);
        }
    }

    return false;
}
