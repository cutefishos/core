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

#include "cpumanagement.h"
#include "cpumanagementadaptor.h"

#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QFile>

CPUManagement::CPUManagement(QObject *parent)
    : QObject(parent)
    , m_currentMode(PowerSave)
{
    new CPUManagementAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/CPUManagement"), this);

    // Init cpu items.
    QDir dir("/sys/devices/system/cpu");
    QStringList dirList = dir.entryList(QDir::Dirs | QDir::NoDot | QDir::NoDotAndDotDot);

    for (const QString &dirName : dirList) {
        if (dirName.startsWith("cpu") && dirName[3].isNumber()) {
            m_items.append(new CpuItem(dirName));
        }
    }

    // Init mode
    bool performance = false;
    for (CpuItem *item : m_items) {
        qDebug() << item->policy();
        if (item->policy() == "performance") {
            performance = true;
            break;
        }
    }
    if (performance)
        m_currentMode = CPUManagement::Performance;
}

void CPUManagement::setMode(int value)
{
    CPUManagement::Mode mode = static_cast<CPUManagement::Mode>(value);
    QString modeString = modeConvertToString(mode);

    if (modeString.isEmpty() || mode == m_currentMode)
        return;

    QProcess process;
    for (int i = 0; i <= m_items.count(); ++i) {
        process.start("pkexec", QStringList() << "cutefish-cpufreq"
                                              << "-s" << "-c" << QString::number(i)
                                              << "-m" << modeString);
        process.waitForFinished(-1);
    }

    m_currentMode = mode;
    emit modeChanged();
}

int CPUManagement::mode()
{
    return m_currentMode;
}

QString CPUManagement::modeConvertToString(CPUManagement::Mode mode)
{
    QString result;

    switch (mode) {
    case CPUManagement::PowerSave:
        result = "powersave";
        break;
    case CPUManagement::Performance:
        result = "performance";
        break;
    default:
        break;
    }

    return result;
}
