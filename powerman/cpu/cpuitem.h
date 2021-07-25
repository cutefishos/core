/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#ifndef CPUITEM_H
#define CPUITEM_H

#include <QObject>

class CpuItem : public QObject
{
    Q_OBJECT

public:
    explicit CpuItem(const QString &name, QObject *parent = nullptr);

    QString name();

    bool setPolicy(const QString &value);
    QString policy();

private:
    QString m_name;
};

#endif // CPUITEM_H
