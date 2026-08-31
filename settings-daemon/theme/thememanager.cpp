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

#include "thememanager.h"
#include "themeadaptor.h"

#include <KScreen/Config>
#include <KScreen/GetConfigOperation>
#include <KScreen/Mode>
#include <KScreen/Output>
#include <KScreen/SetConfigOperation>

#include <QDomDocument>
#include <QTextStream>
#include <QDBusInterface>
#include <QProcess>
#include <QFile>
#include <QDebug>
#include <QFontDatabase>
#include <QGuiApplication>
#include <QTimer>

#include <limits>

#include <QtMath>

static const QByteArray s_systemFontName = QByteArrayLiteral("Font");
static const QByteArray s_systemFixedFontName = QByteArrayLiteral("FixedFont");
static const QByteArray s_systemPointFontSize = QByteArrayLiteral("FontSize");
static const QByteArray s_devicePixelRatio = QByteArrayLiteral("PixelRatio");

static bool isWaylandSession()
{
    return QGuiApplication::platformName().contains(QStringLiteral("wayland"), Qt::CaseInsensitive)
        || qEnvironmentVariable("XDG_SESSION_TYPE") == QStringLiteral("wayland")
        || !qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY");
}

static QString displayOutputKey(const KScreen::OutputPtr &output)
{
    if (!output) {
        return QString();
    }

    QString key = output->hashMd5();
    if (key.isEmpty()) {
        key = output->name();
    }
    if (key.isEmpty()) {
        key = QString::number(output->id());
    }
    return key.replace(QLatin1Char('/'), QLatin1Char('_'));
}

static bool hasStoredDisplayConfiguration()
{
    QSettings settings(QSettings::UserScope, QStringLiteral("cutefishos"), QStringLiteral("display"));
    settings.beginGroup(QStringLiteral("Outputs"));
    const bool hasConfiguration = !settings.childGroups().isEmpty();
    settings.endGroup();
    return hasConfiguration;
}

static void saveDisplayConfiguration(const KScreen::ConfigPtr &config)
{
    if (!config) {
        return;
    }

    QSettings settings(QSettings::UserScope, QStringLiteral("cutefishos"), QStringLiteral("display"));
    settings.setValue(QStringLiteral("Version"), 1);
    settings.beginGroup(QStringLiteral("Outputs"));

    const KScreen::OutputList outputs = config->outputs();
    for (auto it = outputs.cbegin(); it != outputs.cend(); ++it) {
        const KScreen::OutputPtr output = it.value();
        if (!output || !output->isConnected()) {
            continue;
        }

        settings.beginGroup(displayOutputKey(output));
        settings.setValue(QStringLiteral("Name"), output->name());
        settings.setValue(QStringLiteral("Enabled"), output->isEnabled());
        settings.setValue(QStringLiteral("Primary"), output->isPrimary());
        settings.setValue(QStringLiteral("PositionX"), output->pos().x());
        settings.setValue(QStringLiteral("PositionY"), output->pos().y());
        settings.setValue(QStringLiteral("Scale"), output->scale());
        settings.setValue(QStringLiteral("Rotation"), static_cast<int>(output->rotation()));

        if (const KScreen::ModePtr mode = output->currentMode()) {
            settings.setValue(QStringLiteral("ModeId"), mode->id());
            settings.setValue(QStringLiteral("ModeWidth"), mode->size().width());
            settings.setValue(QStringLiteral("ModeHeight"), mode->size().height());
            settings.setValue(QStringLiteral("ModeRefreshRate"), mode->refreshRate());
        }
        settings.endGroup();
    }

    settings.endGroup();
    settings.sync();

    KScreen::OutputPtr primary = config->primaryOutput();
    if (!primary) {
        for (const KScreen::OutputPtr &output : config->connectedOutputs()) {
            if (output && output->isEnabled()) {
                primary = output;
                break;
            }
        }
    }
    if (primary) {
        QSettings themeSettings(QSettings::UserScope,
                                QStringLiteral("cutefishos"),
                                QStringLiteral("theme"));
        themeSettings.setValue(s_devicePixelRatio, primary->scale());
        themeSettings.sync();
    }
}

static KScreen::ModePtr storedMode(const KScreen::OutputPtr &output, QSettings &settings)
{
    if (!output) {
        return {};
    }

    const QString modeId = settings.value(QStringLiteral("ModeId")).toString();
    if (!modeId.isEmpty()) {
        if (const KScreen::ModePtr mode = output->mode(modeId)) {
            return mode;
        }
    }

    const QSize storedSize(settings.value(QStringLiteral("ModeWidth"), 0).toInt(),
                           settings.value(QStringLiteral("ModeHeight"), 0).toInt());
    const float storedRefreshRate = settings.value(QStringLiteral("ModeRefreshRate"), 0).toFloat();
    if (!storedSize.isValid()) {
        return {};
    }

    KScreen::ModePtr closestMode;
    float closestDifference = std::numeric_limits<float>::max();
    const KScreen::ModeList modes = output->modes();
    for (auto it = modes.cbegin(); it != modes.cend(); ++it) {
        const KScreen::ModePtr mode = it.value();
        if (!mode || mode->size() != storedSize) {
            continue;
        }

        const float difference = qAbs(mode->refreshRate() - storedRefreshRate);
        if (!closestMode || difference < closestDifference) {
            closestMode = mode;
            closestDifference = difference;
        }
    }
    return closestMode;
}

static QString gtkRc2Path()
{
    return QDir::homePath() + QLatin1String("/.gtkrc-2.0");
}

static QString gtk3SettingsIniPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/gtk-3.0/settings.ini");
}

ThemeManager *ThemeManager::self()
{
    static ThemeManager t;
    return &t;
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QStringLiteral("cutefishos"), QStringLiteral("theme")))
{
    if (isWaylandSession() && qEnvironmentVariableIsEmpty("KSCREEN_BACKEND")) {
        qputenv("KSCREEN_BACKEND", QByteArrayLiteral("kwayland"));
    }

    if (!QFile::exists(m_settings->fileName())) {
        QFile file(m_settings->fileName());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream in(&file);
            in << "";
            file.close();
        }
    }

    // init dbus
    new ThemeAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Theme"), this);

    // init value
    m_isDarkMode = m_settings->value("DarkMode", false).toBool();
    m_darkModeDimsWallpaer = m_settings->value("DarkModeDimsWallpaer", false).toBool();
    m_backgroundVisible = true;
    m_wallpaperPath = m_settings->value("Wallpaper", "/usr/share/backgrounds/cutefishos/default.jpg").toString();
    m_accentColor = m_settings->value("AccentColor", 0).toInt();
    m_backgroundType = m_settings->value("BackgroundType", 0).toInt();
    m_backgroundColor = m_settings->value("BackgroundColor", "#2B8ADA").toString();
    m_cursorTheme = m_settings->value("CursorTheme", "default").toString();
    m_cursorSize = m_settings->value("CursorSize", 24).toInt();
    m_iconTheme = m_settings->value("IconTheme", "Crule").toString();

    // Synchronize toolkit and compositor cursor settings at session startup.
    applyCursorSettings();

    // Init fonts.
    if (!m_settings->contains(s_systemFixedFontName)) {
        m_settings->setValue(s_systemFixedFontName, "Monospace");
    }

    if (!m_settings->contains(s_systemFontName)) {
        QSettings lanSettings(QStringLiteral("cutefishos"), QStringLiteral("language"));
        QString languageCode = lanSettings.value("language").toString();
        QString fontName;

        if (languageCode == "zh_CN") {
            fontName = "Noto Sans CJK SC";
        } else if (languageCode.contains("en_")) {
            fontName = "Noto Sans";
        } else if (languageCode == "zh_TW") {
            fontName = "Noto Sans CJK TC";
        } else if (languageCode == "zh_HK") {
            fontName = "Noto Serif CJK HK";
        } else if (languageCode == "ja_JP") {
            fontName = "Noto Serif CJK JP";
        } else {
            fontName = "Noto Sans";
        }

        m_settings->setValue(s_systemFontName, fontName);
    }

    // 登陆后更新 fontconfig
    updateFontConfig();

    // KWin does not provide the persistence layer for the Cutefish desktop.
    // Restore our display snapshot after the daemon enters its event loop.
    QTimer::singleShot(0, this, &ThemeManager::initializeDisplayConfiguration);
}

bool ThemeManager::isDarkMode()
{
    return m_isDarkMode;
}

void ThemeManager::setDarkMode(bool darkMode)
{
    if (darkMode == m_isDarkMode)
        return;

    m_isDarkMode = darkMode;
    m_settings->setValue("DarkMode", darkMode);

    updateGtk3Config();

    emit darkModeChanged(m_isDarkMode);
}

bool ThemeManager::darkModeDimsWallpaer() const
{
    return m_darkModeDimsWallpaer;
}

void ThemeManager::setDarkModeDimsWallpaer(bool value)
{
    if (m_darkModeDimsWallpaer == value)
        return;

    m_darkModeDimsWallpaer = value;
    m_settings->setValue("DarkModeDimsWallpaer", m_darkModeDimsWallpaer);

    emit darkModeDimsWallpaerChanged();
}

bool ThemeManager::backgroundVisible() const
{
    return m_backgroundVisible;
}

void ThemeManager::setBackgroundVisible(bool value)
{
    if (m_backgroundVisible == value)
        return;

    m_backgroundVisible = value;
    emit backgroundVisibleChanged();
}

int ThemeManager::accentColor()
{
    return m_settings->value("AccentColor", 0).toInt();
}

void ThemeManager::setAccentColor(int accentColor)
{
    if (m_accentColor == accentColor)
        return;
    
    m_accentColor = accentColor;
    m_settings->setValue("AccentColor", m_accentColor);

    emit accentColorChanged(m_accentColor);
}

QString ThemeManager::cursorTheme() const
{
    return m_cursorTheme;
}

void ThemeManager::setCursorTheme(const QString &theme)
{
    if (m_cursorTheme != theme) {
        m_cursorTheme = theme;
        m_settings->setValue("CursorTheme", m_cursorTheme);
        applyCursorSettings();
        emit cursorThemeChanged();
    }
}

int ThemeManager::cursorSize() const
{
    return m_cursorSize;
}

void ThemeManager::setCursorSize(int size)
{
    if (m_cursorSize != size) {
        m_cursorSize = size;
        m_settings->setValue("CursorSize", m_cursorSize);
        applyCursorSettings();
        emit cursorSizeChanged();
    }
}

void ThemeManager::updateGtk2Config()
{

}

QString ThemeManager::systemFont()
{
    return m_settings->value(s_systemFontName, "Noto Sans").toString();
}

void ThemeManager::setSystemFont(const QString &fontFamily)
{
    m_settings->setValue(s_systemFontName, fontFamily);
    updateGtk3Config();
    updateFontConfig();

    emit systemFontChanged();
}

QString ThemeManager::systemFixedFont()
{
    return m_settings->value(s_systemFixedFontName, "Monospace").toString();
}

void ThemeManager::setSystemFixedFont(const QString &fontFamily)
{
    m_settings->setValue(s_systemFixedFontName, fontFamily);

    updateFontConfig();
}

qreal ThemeManager::systemFontPointSize()
{
    return m_settings->value(s_systemPointFontSize, 9).toReal();
}

void ThemeManager::setSystemFontPointSize(qreal fontSize)
{
    m_settings->setValue(s_systemPointFontSize, fontSize);
    updateGtk3Config();
    emit systemFontPointSizeChanged();
}

qreal ThemeManager::devicePixelRatio()
{
    return m_settings->value(s_devicePixelRatio, 1.0).toReal();
}

void ThemeManager::initializeDisplayConfiguration()
{
    if (!isWaylandSession()) {
        emit displayConfigurationReady();
        return;
    }

    KScreen::GetConfigOperation operation(KScreen::ConfigOperation::NoOptions, this);
    if (!operation.exec() || operation.hasError() || !operation.config()) {
        qWarning() << "Unable to read the display configuration during startup:"
                   << operation.errorString();
        emit displayConfigurationReady();
        return;
    }

    const KScreen::ConfigPtr config = operation.config();
    const bool hasStoredConfiguration = hasStoredDisplayConfiguration();
    const qreal legacyScale = devicePixelRatio();
    QSettings settings(QSettings::UserScope, QStringLiteral("cutefishos"), QStringLiteral("display"));
    settings.beginGroup(QStringLiteral("Outputs"));
    const QStringList storedOutputs = settings.childGroups();

    bool changed = false;
    KScreen::OutputPtr storedPrimary;
    const KScreen::OutputList outputs = config->outputs();
    for (auto it = outputs.cbegin(); it != outputs.cend(); ++it) {
        const KScreen::OutputPtr output = it.value();
        if (!output || !output->isConnected()) {
            continue;
        }

        const QString key = displayOutputKey(output);
        if (!hasStoredConfiguration) {
            if (!qFuzzyCompare(output->scale(), legacyScale)) {
                output->setScale(legacyScale);
                changed = true;
            }
            continue;
        }
        if (!storedOutputs.contains(key)) {
            continue;
        }

        settings.beginGroup(key);
        if (settings.contains(QStringLiteral("Enabled"))) {
            const bool enabled = settings.value(QStringLiteral("Enabled")).toBool();
            if (output->isEnabled() != enabled) {
                output->setEnabled(enabled);
                changed = true;
            }
        }

        if (settings.contains(QStringLiteral("PositionX"))
            && settings.contains(QStringLiteral("PositionY"))) {
            const QPoint position(settings.value(QStringLiteral("PositionX")).toInt(),
                                  settings.value(QStringLiteral("PositionY")).toInt());
            if (output->pos() != position) {
                output->setPos(position);
                changed = true;
            }
        }

        if (settings.contains(QStringLiteral("Scale"))) {
            const qreal scale = settings.value(QStringLiteral("Scale")).toReal();
            if (scale > 0 && !qFuzzyCompare(output->scale(), scale)) {
                output->setScale(scale);
                changed = true;
            }
        }

        if (settings.contains(QStringLiteral("Rotation"))) {
            const auto rotation = static_cast<KScreen::Output::Rotation>(
                settings.value(QStringLiteral("Rotation")).toInt());
            if (output->rotation() != rotation) {
                output->setRotation(rotation);
                changed = true;
            }
        }

        if (const KScreen::ModePtr mode = storedMode(output, settings)) {
            if (output->currentModeId() != mode->id()) {
                output->setCurrentModeId(mode->id());
                changed = true;
            }
        }

        if (settings.value(QStringLiteral("Primary"), false).toBool()) {
            storedPrimary = output;
        }
        settings.endGroup();
    }
    settings.endGroup();

    if (storedPrimary && !storedPrimary->isPrimary()) {
        config->setPrimaryOutput(storedPrimary);
        changed = true;
    }

    if (changed) {
        if (!KScreen::Config::canBeApplied(config,
                                           KScreen::Config::ValidityFlag::RequireAtLeastOneEnabledScreen)) {
            qWarning() << "The stored display configuration is not valid";
            emit displayConfigurationReady();
            return;
        }

        KScreen::SetConfigOperation setOperation(config, this);
        if (!setOperation.exec() || setOperation.hasError()) {
            qWarning() << "Unable to restore the stored display configuration through KWin Wayland:"
                       << setOperation.errorString();
            emit displayConfigurationReady();
            return;
        }
    }

    // Capture the current state on first startup and keep the legacy theme
    // scale synchronized for GTK, cursor settings, and already-running tools.
    saveDisplayConfiguration(config);
    m_settings->sync();
    updateGtk3Config();
    applyCursorSettings();
    emit displayConfigurationReady();
}

bool ThemeManager::applyScaleToDisplays(qreal scale)
{
    if (!isWaylandSession()) {
        return false;
    }

    KScreen::GetConfigOperation operation(KScreen::ConfigOperation::NoOptions, this);
    if (!operation.exec() || operation.hasError() || !operation.config()) {
        qWarning() << "Unable to read the display configuration for scaling:"
                   << operation.errorString();
        return false;
    }

    const KScreen::ConfigPtr config = operation.config();
    bool changed = false;
    for (const KScreen::OutputPtr &output : config->connectedOutputs()) {
        if (output && !qFuzzyCompare(output->scale(), scale)) {
            output->setScale(scale);
            changed = true;
        }
    }

    // Persist before applying so a compositor restart cannot lose the user's
    // requested scale even if the live KWin operation fails.
    saveDisplayConfiguration(config);
    if (!changed) {
        return true;
    }

    KScreen::SetConfigOperation setOperation(config, this);
    if (!setOperation.exec() || setOperation.hasError()) {
        qWarning() << "Unable to apply the display scale through KWin Wayland:"
                   << setOperation.errorString();
        return false;
    }
    return true;
}

void ThemeManager::setDevicePixelRatio(qreal ratio)
{
    ratio = qBound<qreal>(1.0, ratio, 4.0);
    m_settings->setValue(s_devicePixelRatio, ratio);
    m_settings->sync();

    applyScaleToDisplays(ratio);
    updateGtk3Config();
    applyCursorSettings();

    QProcess p;
    p.setProgram("pkexec");
    p.setArguments(QStringList() << "cutefish-sddm-helper"
                                 << "--scale" << QString::number(ratio));
    p.start();
    p.waitForFinished(-1);

    QDBusInterface iface("org.freedesktop.Notifications",
                         "/org/freedesktop/Notifications",
                         "org.freedesktop.Notifications",
                         QDBusConnection::sessionBus());
    if (iface.isValid()) {
        QList<QVariant> args;
        args << "cutefish-settings";
        args << ((unsigned int) 0);
        args << "preferences-system";
        args << "";
        args << tr("Screen scaling needs to be re-login to take effect");
        args << QStringList();
        args << QVariantMap();
        args << (int) 10;
        iface.asyncCallWithArgumentList("Notify", args);
    }
}

QString ThemeManager::wallpaper()
{
    return m_wallpaperPath;
}

void ThemeManager::setWallpaper(const QString &path)
{
    if (m_wallpaperPath != path) {
        m_wallpaperPath = path;
        m_settings->setValue("Wallpaper", path);
        emit wallpaperChanged(path);
    }
}

int ThemeManager::backgroundType()
{
    return m_backgroundType;
}

void ThemeManager::setBackgroundType(int type)
{
    if (m_backgroundType != type) {
        m_backgroundType = type;
        m_settings->setValue("BackgroundType", m_backgroundType);
        emit backgroundTypeChanged();
    }
}

QString ThemeManager::backgroundColor()
{
    return m_backgroundColor;
}

void ThemeManager::setBackgroundColor(QString color)
{
    if (m_backgroundColor != color) {
        m_backgroundColor = color;
        m_settings->setValue("BackgroundColor", m_backgroundColor);
        emit backgroundColorChanged();
    }
}

void ThemeManager::updateGtk3Config()
{
    QSettings settings(gtk3SettingsIniPath(), QSettings::IniFormat);
    settings.clear();
    settings.beginGroup("Settings");

    // font
    settings.setValue("gtk-font-name", QString("%1 %2").arg(systemFont()).arg(systemFontPointSize()));
    // dark mode
    settings.setValue("gtk-application-prefer-dark-theme", isDarkMode());
    // icon theme
    settings.setValue("gtk-icon-theme-name", m_iconTheme);
    settings.setValue("gtk-cursor-theme-name", m_cursorTheme);
    settings.setValue("gtk-cursor-theme-size", qRound(m_cursorSize * devicePixelRatio()));
    // other
    settings.setValue("gtk-enable-animations", true);
    // theme
    settings.setValue("gtk-theme-name", isDarkMode() ? "Cutefish-dark" : "Cutefish-light");
    settings.sync();
}

void ThemeManager::applyFontSettings()
{
    m_settings->sync();
    updateFontConfig();
    updateGtk3Config();
}

void ThemeManager::applyCursorSettings()
{
    m_settings->sync();

    QSettings inputSettings(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation)
                                + QStringLiteral("/kcminputrc"),
                            QSettings::IniFormat);
    inputSettings.beginGroup(QStringLiteral("Mouse"));
    inputSettings.setValue(QStringLiteral("cursorTheme"), cursorTheme());
    inputSettings.setValue(QStringLiteral("cursorSize"),
                           qRound(cursorSize() * devicePixelRatio()));
    inputSettings.endGroup();
    inputSettings.sync();

    updateGtk3Config();

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings",
                                                      "org.kde.KGlobalSettings",
                                                      "notifyChange");
    message << 5;
    message << 0;
    QDBusConnection::sessionBus().send(message);

    QDBusInterface kwin(QStringLiteral("org.kde.KWin"), QStringLiteral("/KWin"),
                        QStringLiteral("org.kde.KWin"), QDBusConnection::sessionBus());
    if (kwin.isValid())
        kwin.asyncCall(QStringLiteral("reconfigure"));
}

void ThemeManager::updateFontConfig()
{
    QFontDatabase database;
    const QStringList families = database.families();
    const QString familyFont = families.contains(systemFont())
            ? systemFont()
            : families.contains(QStringLiteral("Noto Sans"))
                ? QStringLiteral("Noto Sans")
                : QStringLiteral("DejaVu Sans");
    const QString fixedFont = families.contains(systemFixedFont())
            ? systemFixedFont()
            : families.contains(QStringLiteral("Noto Sans Mono"))
                ? QStringLiteral("Noto Sans Mono")
                : QStringLiteral("DejaVu Sans Mono");
    const QString familyFallback = familyFont;

    QSettings settings(QSettings::UserScope, "cutefishos", "theme");
    bool hinting = settings.value("FontAntialias", true).toBool();
    QString hintStyle = settings.value("FontHintStyle", "hintslight").toString();

    QString content = QString("<?xml version=\"1.0\"?>"
                        "<!DOCTYPE fontconfig SYSTEM \"fonts.dtd\">"
                        "<fontconfig>"
                        "<match target=\"pattern\">"
                        "<test qual=\"any\" name=\"family\">"
                        "<string>serif</string>"
                        "</test>"
                        "<edit name=\"family\" mode=\"assign\" binding=\"strong\">"
                        "<string>%1</string>"
                        "<string>%2</string>"
                        "</edit>"
                        "</match>"
                        "<match target=\"pattern\">"
                        "<test qual=\"any\" name=\"family\">"
                        "<string>sans-serif</string>"
                        "</test>"
                        "<edit name=\"family\" mode=\"assign\" binding=\"strong\">"
                        "<string>%3</string>"
                        "<string>%4</string>"
                        "</edit>"
                        "</match>"
                        "<match target=\"pattern\">"
                        "<test qual=\"any\" name=\"family\">"
                        "<string>monospace</string>"
                        "</test>"
                        "<edit name=\"family\" mode=\"assign\" binding=\"strong\">"
                        "<string>%5</string>"
                        "<string>%6</string>"
                        "<string>%7</string>"
                        "</edit>"
                        "</match>"
                        "<match target=\"font\">"
                        "<edit name=\"rgba\" mode=\"assign\"><const>rgb</const></edit>"
                        "</match>"
                        "<match target=\"font\">"
                        "<edit name=\"hinting\" mode=\"assign\">"
                        "<bool>%8</bool>"
                        "</edit>"
                        "</match>"
                        "<match target=\"font\">"
                        "<edit name=\"hintstyle\" mode=\"assign\">"
                        "<const>%9</const>"
                        "</edit>"
                        "</match>"
                        "</fontconfig>"
    ).arg(familyFont).arg(familyFallback)
     .arg(familyFont).arg(familyFallback)
     .arg(fixedFont).arg(fixedFont)
     .arg(familyFont).arg(hinting ? "true" : "false")
     .arg(hintStyle);

    QString targetPath(QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1Char('/') + QLatin1String("fontconfig"));

    if (!QDir(targetPath).exists()) {
        QDir(targetPath).mkpath(targetPath);
    }

    targetPath += "/conf.d";

    if (!QDir(targetPath).exists()) {
        QDir(targetPath).mkpath(targetPath);
    }

    QString xmlOut;
    QXmlStreamReader reader(content);
    QXmlStreamWriter writer(&xmlOut);
    writer.setAutoFormatting(true);

    while (!reader.atEnd()) {
        reader.readNext();
        if (!reader.isWhitespace()) {
            writer.writeCurrentToken(reader);
        }
    }

    QFile file(targetPath + "/99-cutefish.conf");
    if (file.open(QIODevice::WriteOnly)) {
        QTextStream s(&file);
        s << xmlOut.toLatin1();
        file.close();
    }
}

QString ThemeManager::iconTheme() const
{
    return m_iconTheme;
}

void ThemeManager::setIconTheme(const QString &iconTheme)
{
    if (m_iconTheme == iconTheme)
        return;

    m_iconTheme = iconTheme;
    m_settings->setValue("IconTheme", m_iconTheme);
    updateGtk3Config();
    emit iconThemeChanged();
}
