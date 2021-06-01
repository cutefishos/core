/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     rekols <revenmartin@gmail.com>
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

#include "dock.h"
#include "dockadaptor.h"

#include <QDBusConnection>
#include <QDBusServiceWatcher>
#include <QDBusInterface>

#include <QFile>
#include <QDebug>

Dock::Dock(QObject *parent)
    : QObject(parent)
    , m_iconSize(0)
    , m_edgeMargins(10)
    , m_roundedWindowEnabled(true)
    , m_direction(Left)
    , m_visibility(AlwaysVisible)
    , m_settings(new QSettings(QSettings::UserScope, "cutefishos", "dock"))
{
    new DockAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Dock"), this);

    if (!m_settings->contains("IconSize"))
        m_settings->setValue("IconSize", 64);
    if (!m_settings->contains("Direction"))
        m_settings->setValue("Direction", Bottom);
    if (!m_settings->contains("Visibility"))
        m_settings->setValue("Visibility", AlwaysVisible);
    if (!m_settings->contains("RoundedWindow"))
        m_settings->setValue("RoundedWindow", true);

    m_settings->sync();

    m_iconSize = m_settings->value("IconSize").toInt();
    m_direction = static_cast<Direction>(m_settings->value("Direction").toInt());
    m_roundedWindowEnabled = m_settings->value("RoundedWindow").toBool();
}

int Dock::iconSize() const
{
    return m_iconSize;
}

void Dock::setIconSize(int iconSize)
{
    if (m_iconSize != iconSize) {
        m_iconSize = iconSize;
        m_settings->setValue("IconSize", iconSize);
        emit iconSizeChanged();
    }
}

Dock::Direction Dock::direction() const
{
    return m_direction;
}

void Dock::setDirection(int value)
{
    if (m_direction != value) {
        m_direction = static_cast<Direction>(value);
        m_settings->setValue("Direction", value);
        emit directionChanged();
    }
}

Dock::Visibility Dock::visibility() const
{
    return m_visibility;
}

void Dock::setVisibility(int value)
{
    if (m_visibility != value) {
        m_visibility = static_cast<Visibility>(value);
        m_settings->setValue("Visibility", value);
        emit visibilityChanged();
    }
}

int Dock::edgeMargins() const
{
    return m_edgeMargins;
}

void Dock::setEdgeMargins(int edgeMargins)
{
    m_edgeMargins = edgeMargins;
}

bool Dock::roundedWindowEnabled() const
{
    return m_roundedWindowEnabled;
}

void Dock::setRoundedWindowEnabled(bool enabled)
{
    if (m_roundedWindowEnabled != enabled) {
        m_roundedWindowEnabled = enabled;
        emit roundedWindowEnabledChanged();
    }
}
