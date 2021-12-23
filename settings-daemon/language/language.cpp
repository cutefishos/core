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

#include "language.h"
#include "languageadaptor.h"
#include "theme/thememanager.h"

#include <QDBusInterface>
#include <QSettings>

Language *Language::self()
{
    static Language s;
    return &s;
}

Language::Language(QObject *parent)
    : QObject(parent)
    , m_settings(new QSettings(QStringLiteral("cutefishos"), QStringLiteral("language")))
{
    new LanguageAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Language"), this);

    if (!m_settings->contains("language"))
        m_settings->setValue("language", "en_US");

    emit languageChanged();
}

QString Language::languageCode() const
{
    return m_settings->value("language").toString();
}

void Language::setLanguage(const QString &code)
{
    if (m_settings->value("language").toString() == code) {
        return;
    }

    QSettings settings(QStringLiteral("cutefishos"), QStringLiteral("theme"));

    // Change fonts
    if (code == "zh_CN") {
        settings.setValue("Font", "Noto Sans CJK SC");
    } else if (code.contains("en_")) {
        settings.setValue("Font", "Noto Sans");
    } else if (code == "zh_TW") {
        settings.setValue("Font", "Noto Sans CJK TC");
    } else if (code == "zh_HK") {
        settings.setValue("Font", "Noto Serif CJK HK");
    } else if (code == "ja_JP") {
        settings.setValue("Font", "Noto Serif CJK JP");
    }

    m_settings->setValue("language", code);
    emit languageChanged();

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
        args << tr("The system language has been changed, please log out and log in");
        args << QStringList();
        args << QVariantMap();
        args << (int) 10;
        iface.asyncCallWithArgumentList("Notify", args);
    }
}
