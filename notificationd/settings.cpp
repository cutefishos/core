/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <kateleet@cutefishos.com>
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

#include "settings.h"

Settings *Settings::self()
{
    static Settings s;
    return &s;
}

Settings::Settings(QObject *parent)
    : QObject(parent)
    , m_settings(QSettings::UserScope, "cutefishos", "notification")
{
    m_doNotDisturb = m_settings.value("DoNotDisturb", false).toBool();
}

bool Settings::doNotDisturb() const
{
    return m_doNotDisturb;
}

void Settings::setDoNotDisturb(bool doNotDisturb)
{
    if (m_doNotDisturb != doNotDisturb) {
        m_doNotDisturb = doNotDisturb;

        m_settings.setValue("DoNotDisturb", m_doNotDisturb);

        emit doNotDisturbChanged();
    }
}
