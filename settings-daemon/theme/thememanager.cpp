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

#include <QDomDocument>
#include <QTextStream>
#include <QDBusInterface>
#include <QProcess>
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

ThemeManager *ThemeManager::self()
{
    static ThemeManager t;
    return &t;
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
    m_cursorTheme = m_settings->value("CursorTheme", "default").toString();
    m_cursorSize = m_settings->value("CursorSize", 24).toInt();
    m_iconTheme = m_settings->value("IconTheme", "Crule").toString();

    // Start the DE and need to update the settings again.
    updateGtk3Config();

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
        applyXResources();
        applyCursor();
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
        applyXResources();
        applyCursor();
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

void ThemeManager::setDevicePixelRatio(qreal ratio)
{
    int fontDpi = qRound(ratio * 96.0);
    m_settings->setValue(s_devicePixelRatio, ratio);
    m_settings->setValue("forceFontDPI", fontDpi);
    m_settings->sync();
    applyXResources();

    // SDDM
    QProcess p;
    p.setProgram("pkexec");
    p.setArguments(QStringList() << "cutefish-sddm-helper"
                                 << "--dpi" << QString::number(fontDpi));
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
    settings.setIniCodec("UTF-8");
    settings.beginGroup("Settings");

    // font
    settings.setValue("gtk-font-name", QString("%1 %2").arg(systemFont()).arg(systemFontPointSize()));
    // dark mode
    settings.setValue("gtk-application-prefer-dark-theme", isDarkMode());
    // icon theme
    settings.setValue("gtk-icon-theme-name", m_iconTheme);
    // other
    settings.setValue("gtk-enable-animations", true);
    // theme
    settings.setValue("gtk-theme-name", isDarkMode() ? "Cutefish-dark" : "Cutefish-light");
    settings.sync();
}

void ThemeManager::applyXResources()
{
    m_settings->sync();

    qreal scaleFactor = this->devicePixelRatio();
    int fontDpi = 96 * scaleFactor;

    int xftAntialias = m_settings->value("XftAntialias", 1).toBool();
    QString xftHintStyle = m_settings->value("XftHintStyle", "hintfull").toString();

    const QString datas = QString("Xft.dpi: %1\n"
                                  "Xcursor.theme: %2\n"
                                  "Xcursor.size: %3\n"
                                  "Xft.antialias: %4\n"
                                  "Xft.hintstyle: %5")
                          .arg(fontDpi)
                          .arg(m_cursorTheme)
                          .arg(m_cursorSize * scaleFactor)
                          .arg(xftAntialias)
                          .arg(xftHintStyle);

    QProcess p;
    p.start(QStringLiteral("xrdb"), {QStringLiteral("-quiet"), QStringLiteral("-merge"), QStringLiteral("-nocpp")});
    p.setProcessChannelMode(QProcess::ForwardedChannels);
    p.write(datas.toLatin1());
    p.closeWriteChannel();
    p.waitForFinished(-1);
}

void ThemeManager::applyCursor()
{
    QProcess p;
    p.start("cupdatecursor", QStringList() << cursorTheme() << QString::number(cursorSize() * devicePixelRatio()));
    p.waitForFinished(-1);

    QDBusMessage message = QDBusMessage::createSignal("/KGlobalSettings",
                                                      "org.kde.KGlobalSettings",
                                                      "notifyChange");
    // ChangeCursor
    message << 5;
    message << 0;
    QDBusConnection::sessionBus().send(message);
}

void ThemeManager::updateFontConfig()
{
    const QString &familyFont = systemFont();
    const QString &fixedFont = systemFixedFont();

    const QString &familyFallback = "Noto Sans";

    QSettings settings(QSettings::UserScope, "cutefishos", "theme");
    bool hinting = settings.value("XftAntialias", 1).toBool();
    QString hintStyle = settings.value("XftHintStyle", "hintslight").toString();

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
