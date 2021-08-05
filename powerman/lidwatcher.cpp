#include "lidwatcher.h"
#include <QDebug>

LidWatcher::LidWatcher(QObject *parent)
    : QObject(parent)
{
    m_uPowerIface = new QDBusInterface("org.freedesktop.UPower",
                                       "/org/freedesktop/UPower",
                                       "org.freedesktop.UPower", QDBusConnection::systemBus(), this);

    m_uPowerPropertiesIface = new QDBusInterface("org.freedesktop.UPower",
                                                 "/org/freedesktop/UPower",
                                                 "org.freedesktop.DBus.Properties",
                                                 QDBusConnection::systemBus(), this);

    QDBusConnection::systemBus().connect("org.freedesktop.UPower",
                                         "/org/freedesktop/UPower",
                                         "org.freedesktop.DBus.Properties",
                                         "PropertiesChanged",
                                         this, SLOT(onPropertiesChanged()));

    m_lidClosed = m_uPowerPropertiesIface->property("LidIsClosed").toBool();
}

bool LidWatcher::existLid() const
{
    return m_uPowerIface->property("LidIsPresent").toBool();
}

bool LidWatcher::lidClosed() const
{
    return m_lidClosed;
}

void LidWatcher::onPropertiesChanged()
{
    bool isLidClosed = m_uPowerIface->property("LidIsClosed").toBool();

    if (isLidClosed != m_lidClosed) {
        m_lidClosed = isLidClosed;
        emit lidClosedChanged();
    }
}
