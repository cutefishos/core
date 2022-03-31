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

// Qt
#include <QCommandLineOption>
#include <QDBusConnection>
#include <QStandardPaths>
#include <QSettings>
#include <QProcess>
#include <QDebug>
#include <QDir>

#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

// STL
#include <optional>

std::optional<QStringList> getSystemdEnvironment()
{
    QStringList list;
    auto msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.systemd1"),
                                              QStringLiteral("/org/freedesktop/systemd1"),
                                              QStringLiteral("org.freedesktop.DBus.Properties"),
                                              QStringLiteral("Get"));
    msg << QStringLiteral("org.freedesktop.systemd1.Manager") << QStringLiteral("Environment");
    auto reply = QDBusConnection::sessionBus().call(msg);
    if (reply.type() == QDBusMessage::ErrorMessage) {
        return std::nullopt;
    }

    // Make sure the returned type is correct.
    auto arguments = reply.arguments();
    if (arguments.isEmpty() || arguments[0].userType() != qMetaTypeId<QDBusVariant>()) {
        return std::nullopt;
    }
    auto variant = qdbus_cast<QVariant>(arguments[0]);
    if (variant.type() != QVariant::StringList) {
        return std::nullopt;
    }

    return variant.toStringList();
}

bool isShellVariable(const QByteArray &name)
{
    return name == "_" || name.startsWith("SHLVL");
}

bool isSessionVariable(const QByteArray &name)
{
    // Check is variable is specific to session.
    return name == "DISPLAY" || name == "XAUTHORITY" || //
        name == "WAYLAND_DISPLAY" || name == "WAYLAND_SOCKET" || //
        name.startsWith("XDG_");
}

void setEnvironmentVariable(const QByteArray &name, const QByteArray &value)
{
    if (qgetenv(name) != value) {
        qputenv(name, value);
    }
}

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv)
    , m_processManager(new ProcessManager(this))
    , m_networkProxyManager(new NetworkProxyManager)
    , m_wayland(false)
{
    new SessionAdaptor(this);

    // connect to D-Bus and register as an object:
    QDBusConnection::sessionBus().registerService(QStringLiteral("com.cutefish.Session"));
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Session"), this);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Cutefish Session"));
    parser.addHelpOption();

    QCommandLineOption waylandOption(QStringList() << "w" << "wayland" << "Wayland Mode");
    parser.addOption(waylandOption);
    parser.process(*this);

    m_wayland = parser.isSet(waylandOption);

    createConfigDirectory();
    initKWinConfig();
    initLanguage();
    initScreenScaleFactors();
    initXResource();

    initEnvironments();

    if (!syncDBusEnvironment()) {
        // Startup error
        qDebug() << "Could not sync environment to dbus.";
        qApp->exit(1);
    }

    // We import systemd environment after we sync the dbus environment here.
    // Otherwise it may leads to some unwanted order of applying environment
    // variables (e.g. LANG and LC_*)
    // ref plasma
    importSystemdEnvrionment();

    qunsetenv("XCURSOR_THEME");
    qunsetenv("XCURSOR_SIZE");
    qunsetenv("SESSION_MANAGER");

    m_networkProxyManager->update();

    QTimer::singleShot(50, this, &Application::updateUserDirs);
    QTimer::singleShot(100, m_processManager, &ProcessManager::start);
}

bool Application::wayland() const
{
    return m_wayland;
}

void Application::launch(const QString &exec, const QStringList &args)
{
    QProcess process;
    process.setProgram(exec);
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    process.setArguments(args);
    process.startDetached();
}

void Application::launch(const QString &exec, const QString &workingDir, const QStringList &args)
{
    QProcess process;
    process.setProgram(exec);
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    process.setWorkingDirectory(workingDir);
    process.setArguments(args);
    process.startDetached();
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
    
    // ref: https://stackoverflow.com/questions/34399993/qml-performance-issue-when-updating-an-item-in-presence-of-many-non-overlapping
    qputenv("QT_QPA_UPDATE_IDLE_TIME", "10");

    qputenv("QT_AUTO_SCREEN_SCALE_FACTOR", "0");

    // IM Config
    // qputenv("GTK_IM_MODULE", "fcitx5");
    // qputenv("QT4_IM_MODULE", "fcitx5");
    // qputenv("QT_IM_MODULE", "fcitx5");
    // qputenv("CLUTTER_IM_MODULE", "fcitx5");
    // qputenv("XMODIFIERS", "@im=fcitx");
}

void Application::initLanguage()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "language");
    QString value = settings.value("language", "").toString();

    // Init Language
    if (value.isEmpty()) {
        QFile file("/etc/locale.gen");
        if (file.open(QIODevice::ReadOnly)) {
            QStringList lines = QString(file.readAll()).split('\n');

            for (const QString &line : lines) {
                if (line.startsWith('#'))
                    continue;

                if (line.trimmed().isEmpty())
                    continue;

                value = line.split(' ').first().split('.').first();
            }
        }
    }

    if (value.isEmpty())
        value = "en_US";

    settings.setValue("language", value);

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

void Application::initScreenScaleFactors()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "theme");
    qreal scaleFactor = settings.value("PixelRatio", 1.0).toReal();

    qputenv("QT_SCREEN_SCALE_FACTORS", QByteArray::number(scaleFactor));

    // for Gtk
    if (qFloor(scaleFactor) > 1) {
        qputenv("GDK_SCALE", QByteArray::number(scaleFactor, 'g', 0));
        qputenv("GDK_DPI_SCALE", QByteArray::number(1.0 / scaleFactor, 'g', 3));
    } else {
        qputenv("GDK_SCALE", QByteArray::number(qFloor(scaleFactor), 'g', 0));
        qputenv("GDK_DPI_SCALE", QByteArray::number(qFloor(scaleFactor), 'g', 0));
    }
}

void Application::initXResource()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "theme");
    qreal scaleFactor = settings.value("PixelRatio", 1.0).toReal();
    int fontDpi = 96 * scaleFactor;
    QString cursorTheme = settings.value("CursorTheme", "default").toString();
    int cursorSize = settings.value("CursorSize", 24).toInt() * scaleFactor;
    int xftAntialias = settings.value("XftAntialias", 1).toBool();
    QString xftHintStyle = settings.value("XftHintStyle", "hintslight").toString();

    const QString datas = QString("Xft.dpi: %1\n"
                                  "Xcursor.theme: %2\n"
                                  "Xcursor.size: %3\n"
                                  "Xft.antialias: %4\n"
                                  "Xft.hintstyle: %5\n"
                                  "Xft.rgba: rgb")
                          .arg(fontDpi)
                          .arg(cursorTheme)
                          .arg(cursorSize)
                          .arg(xftAntialias)
                          .arg(xftHintStyle);

    QProcess p;
    p.start(QStringLiteral("xrdb"), {QStringLiteral("-quiet"), QStringLiteral("-merge"), QStringLiteral("-nocpp")});
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.write(datas.toLatin1());
    p.closeWriteChannel();
    p.waitForFinished(-1);

    // For cutefish-wine
    qputenv("CUTEFISH_FONT_DPI", QByteArray::number(fontDpi));

    // Init cursor
    runSync("cupdatecursor", {cursorTheme, QString::number(cursorSize)});
    // qputenv("XCURSOR_THEME", cursorTheme.toLatin1());
    // qputenv("XCURSOR_SIZE", QByteArray::number(cursorSize * scaleFactor));
}

void Application::initKWinConfig()
{
    QSettings settings(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/kwinrc",
                       QSettings::IniFormat);

    settings.beginGroup("Effect-Blur");
    settings.setValue("BlurStrength", 10);
    settings.setValue("NoiseStrength", 0);
    settings.endGroup();

    settings.beginGroup("Windows");
    settings.setValue("FocusStealingPreventionLevel", 0);
    settings.setValue("HideUtilityWindowsForInactive", false);
    settings.setValue("BorderlessMaximizedWindows", false);
    settings.setValue("Placement", "Centered");
    settings.endGroup();

    settings.beginGroup("org.kde.kdecoration2");
    settings.setValue("BorderSize", "Normal");
    settings.setValue("ButtonsOnLeft", "");
    settings.setValue("ButtonsOnRight", "HIAX");
    settings.setValue("library", "org.cutefish.decoration");
    settings.setValue("theme", "");
    settings.endGroup();
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

// Import systemd user environment.
// Systemd read ~/.config/environment.d which applies to all systemd user unit.
// But it won't work if cutefishDE is not started by systemd.
void Application::importSystemdEnvrionment()
{
    auto environment = getSystemdEnvironment();
    if (!environment) {
        return;
    }

    for (auto &envString : environment.value()) {
        const auto env = envString.toLocal8Bit();
        const int idx = env.indexOf('=');
        if (Q_UNLIKELY(idx <= 0)) {
            continue;
        }

        const auto name = env.left(idx);
        if (isShellVariable(name) || isSessionVariable(name)) {
            continue;
        }
        setEnvironmentVariable(name, env.mid(idx + 1));
    }
}

void Application::createConfigDirectory()
{
    const QString configDir = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if (!QDir().mkpath(configDir))
        qDebug() << "Could not create config directory XDG_CONFIG_HOME: " << configDir;
}

void Application::updateUserDirs()
{
    // bool isCutefishOS = QFile::exists("/etc/cutefishos");

    // if (!isCutefishOS)
    //     return;

    // QProcess p;
    // p.setEnvironment(QStringList() << "LC_ALL=C");
    // p.start("xdg-user-dirs-update", QStringList() << "--force");
    // p.waitForFinished(-1);
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
