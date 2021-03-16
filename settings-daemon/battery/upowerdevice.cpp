#include "upowerdevice.h"
#include "power.h"

#include <QDebug>
#include <QStringList>
#include <QDBusReply>

UPowerDevice::UPowerDevice(const QString &udi, QObject *parent)
    : QObject(parent)
    , m_device(UP_DBUS_SERVICE,
               udi,
               UP_DBUS_INTERFACE_DEVICE,
               QDBusConnection::systemBus())
    , m_udi(udi)
{
    if (m_device.isValid()) {
        if (m_device.metaObject()->indexOfSignal("Changed()") != -1) {
            connect(&m_device, SIGNAL(Changed()), this, SLOT(slotChanged()));
        } else {
            // for UPower >= 0.99.0, missing Changed() signal
            QDBusConnection::systemBus().connect(UP_DBUS_SERVICE, m_udi, "org.freedesktop.DBus.Properties",
                                                 "PropertiesChanged", this,
                                                 SLOT(onPropertiesChanged(QString, QVariantMap, QStringList)));
        }

        // TODO port this to Solid::Power, we can't link against kdelibs4support for this signal
        // older upower versions not affected
        QDBusConnection::systemBus().connect("org.freedesktop.login1", "/org/freedesktop/login1", "org.freedesktop.login1.Manager", "PrepareForSleep",
                                             this, SLOT(login1Resuming(bool)));
    }
}

bool UPowerDevice::queryDeviceInterface(UPowerDevice::Type type) const
{
    const uint uptype = prop("Type").toUInt();
    switch (type) {
    case GenericInterface:
        return true;
    case Battery:
        switch (uptype) {
        case UP_DEVICE_KIND_BATTERY:
        case UP_DEVICE_KIND_UPS:
        case UP_DEVICE_KIND_MOUSE:
        case UP_DEVICE_KIND_KEYBOARD:
        case UP_DEVICE_KIND_PDA:
        case UP_DEVICE_KIND_PHONE:
        case UP_DEVICE_KIND_GAMING_INPUT:
            return true;
        case UP_DEVICE_KIND_UNKNOWN:
            // There is currently no "Bluetooth battery" type, so check if it comes from Bluez
            if (prop("NativePath").toString().startsWith(QLatin1String("/org/bluez/"))) {
                return true;
            }
            return false;
        case UP_DEVICE_KIND_LINE_POWER:
        case UP_DEVICE_KIND_MONITOR:
        case UP_DEVICE_KIND_MEDIA_PLAYER:
        case UP_DEVICE_KIND_TABLET:
        case UP_DEVICE_KIND_COMPUTER:
        case UP_DEVICE_KIND_LAST:
            return false;
        }
    default:
        return false;
    }
}

QString UPowerDevice::description() const
{
    if (queryDeviceInterface(Battery)) {
        return tr("%1 Battery", "%1 is battery technology").arg(batteryTechnology());
    } else {
        QString result = prop("Model").toString();
        if (result.isEmpty()) {
            return vendor();
        }
        return result;
    }
}

QString UPowerDevice::batteryTechnology() const
{
    const uint tech = prop("Technology").toUInt();

    switch (tech) {
    case 1:
        return tr("Lithium-ion", "battery technology");
    case 2:
        return tr("Lithium Polymer", "battery technology");
    case 3:
        return tr("Lithium Iron Phosphate", "battery technology");
    case 4:
        return tr("Lead Acid", "battery technology");
    case 5:
        return tr("Nickel Cadmium", "battery technology");
    case 6:
        return tr("Nickel Metal Hydride", "battery technology");
    default:
        return tr("Unknown", "battery technology");
    }
}

QString UPowerDevice::product() const
{
    QString result = prop("Model").toString();

    if (result.isEmpty()) {
        result = description();
    }

    return result;
}

QString UPowerDevice::vendor() const
{
    return prop("Vendor").toString();
}

QString UPowerDevice::udi() const
{
    return m_udi;
}

QVariant UPowerDevice::prop(const QString &key) const
{
    checkCache(key);
    return m_cache.value(key);
}

bool UPowerDevice::propertyExists(const QString &key) const
{
    checkCache(key);
    return m_cache.contains(key);
}

QMap<QString, QVariant> UPowerDevice::allProperties() const
{
    QDBusMessage call = QDBusMessage::createMethodCall(m_device.service(), m_device.path(),
                        "org.freedesktop.DBus.Properties", "GetAll");
    call << m_device.interface();
    QDBusPendingReply< QVariantMap > reply = QDBusConnection::systemBus().asyncCall(call);
    reply.waitForFinished();

    if (reply.isValid()) {
        m_cache = reply.value();
    } else {
        m_cache.clear();
    }

    return m_cache;
}

void UPowerDevice::onPropertiesChanged(const QString &ifaceName, const QVariantMap &changedProps, const QStringList &invalidatedProps)
{
    Q_UNUSED(changedProps);
    Q_UNUSED(invalidatedProps);

    if (ifaceName == UP_DBUS_INTERFACE_DEVICE) {
        slotChanged(); // TODO maybe process the properties separately?
    }
}

void UPowerDevice::login1Resuming(bool active)
{
    if (!active) {
        QDBusReply<void> refreshCall = m_device.asyncCall("Refresh");
        if (refreshCall.isValid()) {
            slotChanged();
        }
    }
}

void UPowerDevice::slotChanged()
{
    m_cache.clear();
    emit changed();
}

void UPowerDevice::checkCache(const QString &key) const
{
    if (m_cache.isEmpty()) { // recreate the cache
        allProperties();
    }

    if (m_cache.contains(key)) {
        return;
    }

    QVariant reply = m_device.property(key.toUtf8());

    if (reply.isValid()) {
        m_cache[key] = reply;
    } else {
        m_cache[key] = QVariant();
    }
}
