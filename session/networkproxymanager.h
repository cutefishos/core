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

#ifndef NETWORKPROXYMANAGER_H
#define NETWORKPROXYMANAGER_H

#include <QObject>
#include <QSettings>

class NetworkProxyManager : public QObject
{
    Q_OBJECT

public:
    explicit NetworkProxyManager(QObject *parent = nullptr);

    void update();

private:
    QSettings m_settings;

    int m_flag;
    bool m_useSameProxy;

    QString m_scriptProxy;
    QString m_httpProxy;
    QString m_ftpProxy;
    QString m_socksProxy;

    QString m_httpProxyPort;
    QString m_ftpProxyPort;
    QString m_socksProxyPort;
};

#endif // NETWORKPROXYMANAGER_H
