/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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

#include "utils.h"
#include <QFile>

Utils::Utils(QObject *parent) : QObject(parent)
{

}

QString Utils::processNameFromPid(uint pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    QString name;

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray cmd = file.readAll();

        if (!cmd.isEmpty()) {
            // extract non-truncated name from cmdline
            int zeroIndex = cmd.indexOf('\0');
            int processNameStart = cmd.lastIndexOf('/', zeroIndex);
            if (processNameStart == -1) {
                processNameStart = 0;
            } else {
                processNameStart++;
            }
            name = QString::fromLocal8Bit(cmd.mid(processNameStart, zeroIndex - processNameStart));
        }
    }

    return name;
}
