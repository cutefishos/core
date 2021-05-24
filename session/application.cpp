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
#include "sessionadaptor.h"

#include <QDBusConnection>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QDir>

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_processManager(new ProcessManager)
{
    new SessionAdaptor(this);

    // connect to D-Bus and register as an object:
    QDBusConnection::sessionBus().registerService(QStringLiteral("org.cutefish.Session"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Session"), this);

    createConfigDirectory();
    initEnvironments();
    initLanguage();
    initScreenScaleFactors();

    if (!syncDBusEnvironment()) {
        // Startup error
        qDebug() << "Could not sync environment to dbus.";
    }

    QTimer::singleShot(100, m_processManager, &ProcessManager::start);
}

void Application::initEnvironments()
{
    // Set defaults
    if (qEnvironmentVariableIsEmpty("XDG_DATA_HOME"))
        qputenv("XDG_DATA_HOME", QDir::home().absoluteFilePath(QStringLiteral(".local/share")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_DESKTOP_DIR"))
        qputenv("XDG_DESKTOP_DIR", QDir::home().absoluteFilePath(QStringLiteral("/Desktop")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_HOME"))
        qputenv("XDG_CONFIG_HOME", QDir::home().absoluteFilePath(QStringLiteral(".config")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_CACHE_HOME"))
        qputenv("XDG_CACHE_HOME", QDir::home().absoluteFilePath(QStringLiteral(".cache")).toLocal8Bit());
    if (qEnvironmentVariableIsEmpty("XDG_DATA_DIRS"))
        qputenv("XDG_DATA_DIRS", "/usr/local/share/:/usr/share/");
    if (qEnvironmentVariableIsEmpty("XDG_CONFIG_DIRS"))
        qputenv("XDG_CONFIG_DIRS", "/etc/xdg");

    // Environment
    qputenv("DESKTOP_SESSION", "Cutefish");
    qputenv("XDG_CURRENT_DESKTOP", "Cutefish");
    qputenv("XDG_SESSION_DESKTOP", "Cutefish");

    // Qt
    qputenv("QT_QPA_PLATFORMTHEME", "cutefish");
    qputenv("QT_PLATFORM_PLUGIN", "cutefish");

    qunsetenv("QT_AUTO_SCREEN_SCALE_FACTOR");
    qunsetenv("QT_SCALE_FACTOR");
    qunsetenv("QT_SCREEN_SCALE_FACTORS");
    qunsetenv("QT_ENABLE_HIGHDPI_SCALING");
    qunsetenv("QT_USE_PHYSICAL_DPI");
    qunsetenv("QT_FONT_DPI");
    qputenv("QT_SCALE_FACTOR_ROUNDING_POLICY", "PassThrough");

    // IM Config
    qputenv("GTK_IM_MODULE", "fcitx5");
    qputenv("QT4_IM_MODULE", "fcitx5");
    qputenv("QT_IM_MODULE", "fcitx5");
    qputenv("CLUTTER_IM_MODULE", "fcitx5");
    qputenv("XMODIFIERS", "@im=fcitx");
}

void Application::initLanguage()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "language");
    QString value = settings.value("language", "en_US").toString();
    QString str = QString("%1.UTF-8").arg(value);

    const auto lcValues = {
        "LANG", "LC_NUMERIC", "LC_TIME", "LC_MONETARY", "LC_MEASUREMENT", "LC_COLLATE", "LC_CTYPE"
    };

    for (auto lc : lcValues) {
        const QString value = str;
        if (!value.isEmpty()) {
            qputenv(lc, value.toUtf8());
        }
    }

    if (!value.isEmpty()) {
        qputenv("LANGUAGE", value.toUtf8());
    }
}

static bool isInteger(double x)
{
    int truncated = (int)x;
    return (x == truncated);
}

void Application::initScreenScaleFactors()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "theme");
    qreal scaleFactor = settings.value("PixelRatio", 1.0).toReal();

    qputenv("QT_SCREEN_SCALE_FACTORS", QByteArray::number(scaleFactor));

    // GDK
    if (!isInteger(scaleFactor)) {
        qunsetenv("GDK_SCALE");
        qputenv("GDK_DPI_SCALE", QByteArray::number(scaleFactor));
    } else {
        qputenv("GDK_SCALE", QByteArray::number(scaleFactor, 'g', 0));
        // Intger scale does not adjust GDK_DPI_SCALE.
        // qputenv("GDK_DPI_SCALE", QByteArray::number(scaleFactor, 'g', 3));
    }
}

bool Application::syncDBusEnvironment()
{
    int exitCode = 0;

    // At this point all environment variables are set, let's send it to the DBus session server to update the activation environment
    if (!QStandardPaths::findExecutable(QStringLiteral("dbus-update-activation-environment")).isEmpty()) {
        exitCode = runSync(QStringLiteral("dbus-update-activation-environment"), { QStringLiteral("--systemd"), QStringLiteral("--all") });
    }

    return exitCode == 0;
}

void Application::createConfigDirectory()
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if (!QDir().mkpath(configDir))
        qDebug() << "Could not create config directory XDG_CONFIG_HOME: " << configDir;
}

int Application::runSync(const QString &program, const QStringList &args, const QStringList &env)
{
    QProcess p;

    if (!env.isEmpty())
        p.setEnvironment(QProcess::systemEnvironment() << env);

    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.start(program, args);
    p.waitForFinished(-1);

    if (p.exitCode()) {
        qWarning() << program << args << "exited with code" << p.exitCode();
    }

    return p.exitCode();
}
