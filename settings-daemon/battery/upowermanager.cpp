/*
    SPDX-FileCopyrightText: 2020 Reven Martin <revenmartin@gmail.com>
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "upowermanager.h"
#include "upowerdevice.h"
#include "battery.h"
#include "power.h"

#include <QDBusReply>
#include <QDebug>
#include <QDBusMetaType>
#include <QDBusConnectionInterface>

UPowerManager::UPowerManager(QObject *parent)
    : QObject(parent)
    , m_interface(UP_DBUS_SERVICE,
                  UP_DBUS_PATH,
                  UP_DBUS_INTERFACE,
                  QDBusConnection::systemBus())
    , m_onBattery(false)
{
    qDBusRegisterMetaType<QList<QDBusObjectPath>>();
    qDBusRegisterMetaType<QVariantMap>();

    bool serviceFound = m_interface.isValid();
    if (!serviceFound) {
        // find out whether it will be activated automatically
        QDBusMessage message = QDBusMessage::createMethodCall("org.freedesktop.DBus",
                               "/org/freedesktop/DBus",
                               "org.freedesktop.DBus",
                               "ListActivatableNames");

        QDBusReply<QStringList> reply = QDBusConnection::systemBus().call(message);
        if (reply.isValid() && reply.value().contains(UP_DBUS_SERVICE)) {
            QDBusConnection::systemBus().interface()->startService(UP_DBUS_SERVICE);
            serviceFound = true;
        }
    }

    if (serviceFound) {
        if (m_interface.metaObject()->indexOfSignal("DeviceAdded(QDBusObjectPath)") != -1) {
            // for UPower >= 0.99.0, changed signature :o/
            connect(&m_interface, SIGNAL(DeviceAdded(QDBusObjectPath)),
                    this, SLOT(onDeviceAdded(QDBusObjectPath)));
            connect(&m_interface, SIGNAL(DeviceRemoved(QDBusObjectPath)),
                    this, SLOT(onDeviceRemoved(QDBusObjectPath)));
        } else {
            connect(&m_interface, SIGNAL(DeviceAdded(QString)),
                    this, SIGNAL(deviceAdded(QString)));
            connect(&m_interface, SIGNAL(DeviceRemoved(QString)),
                    this, SIGNAL(deviceRemoved(QString)));
        }

        // On battery
        QDBusConnection::systemBus().connect(UP_DBUS_SERVICE, UP_DBUS_PATH,
                                             "org.freedesktop.DBus.Properties",
                                             "PropertiesChanged", this,
                                             SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        m_onBattery = m_interface.property("OnBattery").toBool();

        // All devices
        QDBusReply<QList<QDBusObjectPath>> reply = m_interface.call("EnumerateDevices");
        if (!reply.isValid()) {
            qWarning() << Q_FUNC_INFO << " error: " << reply.error().name();
            return;
        }

        const auto pathList = reply.value();
        for (const QDBusObjectPath &path : pathList) {
            UPowerDevice *device = new UPowerDevice(path.path());
            Battery *battery = new Battery(device);

            if (battery->type() != Battery::PrimaryBattery) {
                device->deleteLater();
                battery->deleteLater();
            }
        }
    }
}

QString UPowerManager::udiPrefix() const
{
    return UP_UDI_PREFIX;
}

bool UPowerManager::onBattery() const
{
    return m_onBattery;
}

void UPowerManager::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    Q_UNUSED(ifaceName);
    Q_UNUSED(changedProps);
    Q_UNUSED(invalidatedProps);

    const bool onBattery = m_interface.property("OnBattery").toBool();
    m_onBattery = onBattery;
    emit onBatteryChanged();
}

void UPowerManager::onDeviceAdded(const QDBusObjectPath &path)
{
    qDebug() << "onDeviceAdded()" << path.path();
}

void UPowerManager::onDeviceRemoved(const QDBusObjectPath &path)
{
    qDebug() << "onDeviceRemoved()" << path.path();
}
