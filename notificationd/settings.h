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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>

class Settings : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool doNotDisturb READ doNotDisturb WRITE setDoNotDisturb NOTIFY doNotDisturbChanged)

public:
    static Settings *self();

    explicit Settings(QObject *parent = nullptr);

    bool doNotDisturb() const;
    void setDoNotDisturb(bool doNotDisturb);

signals:
    void doNotDisturbChanged();

private:
    QSettings m_settings;
    bool m_doNotDisturb;
};

#endif // SETTINGS_H
