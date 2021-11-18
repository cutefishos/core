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

#include "networkproxymanager.h"
#include <QDBusPendingCall>
#include <QDBusMessage>
#include <QDBusConnection>

NetworkProxyManager::NetworkProxyManager(QObject *parent)
    : QObject(parent)
    , m_settings("cutefishos", "network")
{
    update();
}

void NetworkProxyManager::update()
{
    qunsetenv("HTTP_PROXY");
    qunsetenv("FTP_PROXY");

    m_settings.sync();

    m_flag = m_settings.value("ProxyFlag", 0).toInt();
    m_useSameProxy = m_settings.value("UseSameProxy", false).toBool();
    m_scriptProxy = m_settings.value("ProxyScriptProxy", "").toString();
    m_httpProxy = m_settings.value("HttpProxy", "").toString();
    m_ftpProxy = m_settings.value("FtpProxy", "").toString();
    m_socksProxy = m_settings.value("SocksProxy", "").toString();
    m_httpProxyPort = m_settings.value("HttpProxyPort", "").toString();
    m_ftpProxyPort = m_settings.value("FtpProxyPort", "").toString();
    m_socksProxyPort = m_settings.value("SocksProxyPort", "").toString();

    QMap<QString, QString> dbusActivationEnv;

    if (m_flag == 0) {
        // No proxy
    } else if (m_flag == 1) {
        // Use proxy auto configuration URL
    } else if (m_flag == 2) {
        // Use manually specified proxy configuration

        QString httpProxy = QString("http://%1:%2/").arg(m_httpProxy).arg(m_httpProxyPort);

        if (!m_httpProxy.isEmpty() && !m_httpProxyPort.isEmpty()) {
            qputenv("HTTP_PROXY", httpProxy.toLatin1());
            dbusActivationEnv.insert("HTTP_PROXY", httpProxy);
        }

        if (m_useSameProxy) {
            if (!m_ftpProxy.isEmpty() && !m_ftpProxyPort.isEmpty()) {
                qputenv("FTP_PROXY", httpProxy.toLatin1());
                dbusActivationEnv.insert("FTP_PROXY", httpProxy);
            }
        } else {
            if (!m_ftpProxy.isEmpty() && !m_ftpProxyPort.isEmpty()) {
                qputenv("FTP_PROXY", QString("http://%1:%2/").arg(m_ftpProxy).arg(m_ftpProxyPort).toLatin1());
                dbusActivationEnv.insert("FTP_PROXY", QString("http://%1:%2/").arg(m_ftpProxy).arg(m_ftpProxyPort).toLatin1());
            }
        }

        // if (!m_socksProxy.isEmpty() && !m_socksProxyPort.isEmpty()) {
        //     qputenv("ALL_PROXY", QString("socks://%1:%2/").arg(m_socksProxy).arg(m_socksProxyPort).toLatin1());
        // }
    }

    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.freedesktop.DBus"),
                                                      QStringLiteral("/org/freedesktop/DBus"),
                                                      QStringLiteral("org.freedesktop.DBus"),
                                                      QStringLiteral("UpdateActivationEnvironment"));
    msg.setArguments({QVariant::fromValue(dbusActivationEnv)});
    QDBusPendingCall reply = QDBusConnection::sessionBus().asyncCall(msg);
    reply.waitForFinished();
}
