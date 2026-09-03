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

#ifndef CPUMANAGEMENT_H
#define CPUMANAGEMENT_H

#include <QObject>
#include "cpuitem.h"

class CPUManagement : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    enum Mode {
        PowerSave = 0,
        Performance,
        Normal,
    };

    explicit CPUManagement(QObject *parent = nullptr);

    void setMode(int i);
    int mode();

    QString modeConvertToString(Mode mode);

signals:
    void modeChanged();

private:
    Mode m_currentMode;
    QList<CpuItem *> m_items;
};

#endif // CPUMANAGEMENT_H
