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
#include <QFile>
#include <QDebug>

static const QByteArray s_systemFontName = QByteArrayLiteral("Font");
static const QByteArray s_systemFixedFontName = QByteArrayLiteral("FixedFont");
static const QByteArray s_systemPointFontSize = QByteArrayLiteral("FontSize");
static const QByteArray s_devicePixelRatio = QByteArrayLiteral("PixelRatio");

static QString gtkRc2Path()
{
    return QDir::homePath() + QLatin1String("/.gtkrc-2.0");
}

static QString gtk3SettingsIniPath()
{
    return QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QLatin1String("/gtk-3.0/settings.ini");
}

ThemeManager::ThemeManager(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QStringLiteral("cutefishos"), QStringLiteral("theme")))
{
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
    m_wallpaperPath = m_settings->value("Wallpaper", "/usr/share/backgrounds/cutefishos/default.jpg").toString();
    m_accentColor = m_settings->value("AccentColor", 0).toInt();
    m_backgroundType = m_settings->value("BackgroundType", 0).toInt();
    m_backgroundColor = m_settings->value("BackgroundColor", "#2B8ADA").toString();

    // Start the DE and need to update the settings again.
    initGtkConfig();
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

    updateGtkDarkTheme();

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

QString ThemeManager::systemFont()
{
    return m_settings->value(s_systemFontName, "Noto Sans").toString();
}

void ThemeManager::setSystemFont(const QString &fontFamily)
{
    m_settings->setValue(s_systemFontName, fontFamily);
    updateGtkFont();
}

QString ThemeManager::systemFixedFont()
{
    return m_settings->value(s_systemFixedFontName, "Monospace").toString();
}

void ThemeManager::setSystemFixedFont(const QString &fontFamily)
{
    m_settings->setValue(s_systemFixedFontName, fontFamily);
}

qreal ThemeManager::systemFontPointSize()
{
    return m_settings->value(s_systemPointFontSize, 10.5).toReal();
}

void ThemeManager::setSystemFontPointSize(qreal fontSize)
{
    m_settings->setValue(s_systemPointFontSize, fontSize);
    updateGtkFont();
}

qreal ThemeManager::devicePixelRatio()
{
    return m_settings->value(s_devicePixelRatio, 1.0).toReal();
}

void ThemeManager::setDevicePixelRatio(qreal ratio)
{
    m_settings->setValue(s_devicePixelRatio, ratio);
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

void ThemeManager::initGtkConfig()
{
    QSettings settings(gtk3SettingsIniPath(), QSettings::IniFormat);
    settings.clear();
    settings.setIniCodec("UTF-8");
    settings.beginGroup("Settings");
    // font
    settings.setValue("gtk-font-name", QString("%1 %2").arg(systemFont()).arg(systemFontPointSize()));
    // dark mode
    settings.setValue("gtk-application-prefer-dark-theme", isDarkMode());
    // other
    settings.setValue("gtk-enable-animations", true);
    settings.sync();
}

void ThemeManager::updateGtkFont()
{
    QSettings settings(gtk3SettingsIniPath(), QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup("Settings");
    settings.setValue("gtk-font-name", QString("%1 %2").arg(systemFont()).arg(systemFontPointSize()));
    settings.sync();
}

void ThemeManager::updateGtkDarkTheme()
{
    QSettings settings(gtk3SettingsIniPath(), QSettings::IniFormat);
    settings.setIniCodec("UTF-8");
    settings.beginGroup("Settings");
    settings.setValue("gtk-application-prefer-dark-theme", isDarkMode());
    settings.sync();
}
