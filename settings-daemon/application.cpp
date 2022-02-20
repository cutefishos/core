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

#include "application.h"
#include "dbusadaptor.h"
#include <QStandardPaths>
#include <QProcess>
#include <QTimer>
#include <QFile>
#include <QDebug>
#include <QDir>
#include <QTranslator>
#include <QLocale>
#include <QTimer>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <stdlib.h>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_themeManager(ThemeManager::self())
    , m_brightnessManager(new BrightnessManager(this))
    , m_upowerManager(new UPowerManager(this))
    , m_language(Language::self())
    , m_mouse(new Mouse)
    , m_touchpad(new TouchpadManager)
    , m_defaultApps(new DefaultApplications)
//    , m_kwinTimer(new QTimer(this))
{
    initTrash();

    new DBusAdaptor(this);
    // connect to D-Bus and register as an object:
    QDBusConnection::sessionBus().registerService(QStringLiteral("com.cutefish.Settings"));

//    m_kwinTimer->setSingleShot(false);
//    m_kwinTimer->setInterval(50);
//    connect(m_kwinTimer, &QTimer::timeout, this, &Application::initKWin);
//    m_kwinTimer->start();

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/cutefish-settings-daemon/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QApplication::instance());
        if (translator->load(qmFilePath)) {
            QApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    QTimer::singleShot(10, this, &Application::invokeDesktopProcess);
}

void Application::invokeDesktopProcess()
{
    // Start desktop UI component.
    QDBusInterface sessionInterface("com.cutefish.Session", "/Session", "com.cutefish.Session",
                                    QDBusConnection::sessionBus());

    if (sessionInterface.isValid()) {
        sessionInterface.call("startDesktopProcess");
    }
}

void Application::initTrash()
{
    const QByteArray trashDir = QString(QDir::homePath() + "/.local/share/Trash").toLatin1();

    if (::mkdir(trashDir, 0700) != 0)
        return;

    struct stat buff;
    uid_t uid = getuid();

    if (::lstat(trashDir, &buff) != 0)
        return;

    if ((buff.st_uid == uid) && // must be owned by user
            ((buff.st_mode & 0777) == 0700)) {
        // check subdirs
        QString infoDir = trashDir + QString::fromLatin1("/info");
        QString filesDir = trashDir + QString::fromLatin1("/files");

        if (!QDir(infoDir).exists())
            QDir().mkdir(infoDir);

        if (!QDir(filesDir).exists())
            QDir().mkdir(filesDir);
    } else {
        ::rmdir(trashDir);
    }
}

//void Application::initKWin()
//{
//    QDBusInterface effect("org.kde.KWin", "/Effects", "org.kde.kwin.Effects",
//                           QDBusConnection::sessionBus());

//    if (effect.isValid() && !effect.lastError().isValid()) {
//        // KWin
//        effect.call("loadEffect", "kwin4_effect_dialogparent");

//        effect.call("unloadEffect", "kwin4_effect_fadingpopups");
//        effect.call("unloadEffect", "kwin4_effect_fade");
//        effect.call("unloadEffect", "kwin4_effect_scale");
//        effect.call("unloadEffect", "kwin4_effect_grayscale");
//        effect.call("unloadEffect", "kwin4_effect_squash");
//        effect.call("unloadEffect", "kwin4_effect_translucency");
//        effect.call("unloadEffect", "magiclamp");

//        effect.call("loadEffect", "cutefish_popups");
//        effect.call("loadEffect", "cutefish_scale");
//        effect.call("loadEffect", "cutefish_squash");

//        m_kwinTimer->stop();
//    }
//}
