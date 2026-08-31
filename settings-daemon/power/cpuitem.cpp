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

#include "cpuitem.h"
#include <QFile>
#include <QDebug>

CpuItem::CpuItem(const QString &name, QObject *parent)
    : QObject(parent)
    , m_name(name)
{
}

bool CpuItem::setPolicy(const QString &value)
{
    bool result = false;

    QFile file(QString("/sys/devices/system/cpu/%1/cpufreq/scaling_governor").arg(m_name));
    if (file.open(QIODevice::WriteOnly)) {
        if (file.write(value.toLatin1()) != -1)
            result = true;
        file.close();
    }

    qDebug() << m_name << "set policy: " << value << result;

    return result;
}

QString CpuItem::policy()
{
    QString result;
    QFile file(QString("/sys/devices/system/cpu/%1/cpufreq/scaling_governor").arg(m_name));

    if (file.open(QIODevice::ReadOnly)) {
        result = file.readAll().simplified();
        file.close();
    }

    return result;
}
