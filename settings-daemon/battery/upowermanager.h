/*
    SPDX-FileCopyrightText: 2020 Reven Martin <revenmartin@gmail.com>
    SPDX-FileCopyrightText: 2010 Michael Zanetti <mzanetti@kde.org>
    SPDX-FileCopyrightText: 2010 Lukas Tinkl <ltinkl@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#ifndef UPOWERMANAGER_H
#define UPOWERMANAGER_H

#include <QObject>
#include <QDBusInterface>

class UPowerDevice;
class Battery;
class UPowerManager : public QObject
{
    Q_OBJECT

public:
    explicit UPowerManager(QObject *parent = nullptr);

    QString udiPrefix() const;

    bool onBattery() const;

signals:
    void onBatteryChanged();

private slots:
    void onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps);
    void onDeviceAdded(const QDBusObjectPath &path);
    void onDeviceRemoved(const QDBusObjectPath &path);

private:
    QDBusInterface m_interface;
    bool m_onBattery;
};

#endif // UPOWERMANAGER_H
