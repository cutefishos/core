#include "kwininputbackend.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDBusServiceWatcher>
#include <QDBusVariant>
#include <QTimer>

namespace
{
constexpr auto s_service = "org.kde.KWin";
constexpr auto s_managerPath = "/org/kde/KWin/InputDevice";
constexpr auto s_managerInterface = "org.kde.KWin.InputDeviceManager";
constexpr auto s_deviceInterface = "org.kde.KWin.InputDevice";
constexpr auto s_propertiesInterface = "org.freedesktop.DBus.Properties";
}

KWinInputBackend::KWinInputBackend(DeviceType type, QObject *parent)
    : QObject(parent)
    , m_type(type)
    , m_serviceWatcher(new QDBusServiceWatcher(QString::fromLatin1(s_service),
                                               QDBusConnection::sessionBus(),
                                               QDBusServiceWatcher::WatchForRegistration
                                                   | QDBusServiceWatcher::WatchForUnregistration,
                                               this))
{
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered,
            this, &KWinInputBackend::refreshDevices);
    connect(m_serviceWatcher, &QDBusServiceWatcher::serviceUnregistered,
            this, &KWinInputBackend::refreshDevices);

    QDBusConnection::sessionBus().connect(QString::fromLatin1(s_service),
                                          QString::fromLatin1(s_managerPath),
                                          QString::fromLatin1(s_managerInterface),
                                          QStringLiteral("deviceAdded"),
                                          this,
                                          SLOT(handleDeviceAdded(QString)));
    QDBusConnection::sessionBus().connect(QString::fromLatin1(s_service),
                                          QString::fromLatin1(s_managerPath),
                                          QString::fromLatin1(s_managerInterface),
                                          QStringLiteral("deviceRemoved"),
                                          this,
                                          SLOT(handleDeviceRemoved(QString)));

    QTimer::singleShot(0, this, &KWinInputBackend::refreshDevices);
}

bool KWinInputBackend::available() const
{
    return !matchingDevicePaths().isEmpty();
}

bool KWinInputBackend::booleanProperty(const QString &name, bool fallback) const
{
    const QStringList paths = matchingDevicePaths();
    if (paths.isEmpty())
        return fallback;

    const QVariant value = readProperty(paths.constFirst(),
                                        QString::fromLatin1(s_deviceInterface), name);
    return value.isValid() ? value.toBool() : fallback;
}

qreal KWinInputBackend::realProperty(const QString &name, qreal fallback) const
{
    const QStringList paths = matchingDevicePaths();
    if (paths.isEmpty())
        return fallback;

    const QVariant value = readProperty(paths.constFirst(),
                                        QString::fromLatin1(s_deviceInterface), name);
    return value.isValid() ? value.toReal() : fallback;
}

void KWinInputBackend::setBooleanProperty(const QString &name, bool value)
{
    for (const QString &path : matchingDevicePaths())
        writeProperty(path, name, value);
}

void KWinInputBackend::setRealProperty(const QString &name, qreal value)
{
    value = qBound<qreal>(-1.0, value, 1.0);
    for (const QString &path : matchingDevicePaths())
        writeProperty(path, name, value);
}

void KWinInputBackend::refreshDevices()
{
    const QVariant value = readProperty(QString::fromLatin1(s_managerPath),
                                        QString::fromLatin1(s_managerInterface),
                                        QStringLiteral("devicesSysNames"));
    const QStringList names = value.isValid() ? value.toStringList() : QStringList();
    if (names == m_deviceNames)
        return;

    m_deviceNames = names;
    emit devicesChanged();
}

void KWinInputBackend::handleDeviceAdded(const QString &sysName)
{
    if (!m_deviceNames.contains(sysName)) {
        m_deviceNames.append(sysName);
        emit devicesChanged();
    }
}

void KWinInputBackend::handleDeviceRemoved(const QString &sysName)
{
    if (m_deviceNames.removeAll(sysName) > 0)
        emit devicesChanged();
}

QStringList KWinInputBackend::matchingDevicePaths() const
{
    QStringList paths;
    for (const QString &name : m_deviceNames) {
        const QString path = devicePath(name);
        if (matches(path))
            paths.append(path);
    }
    return paths;
}

bool KWinInputBackend::matches(const QString &path) const
{
    const bool touchpad = readProperty(path, QString::fromLatin1(s_deviceInterface),
                                       QStringLiteral("touchpad")).toBool();
    if (m_type == DeviceType::Touchpad)
        return touchpad;

    const bool pointer = readProperty(path, QString::fromLatin1(s_deviceInterface),
                                      QStringLiteral("pointer")).toBool();
    return pointer && !touchpad;
}

QVariant KWinInputBackend::readProperty(const QString &path, const QString &interface,
                                        const QString &name) const
{
    QDBusMessage message = QDBusMessage::createMethodCall(QString::fromLatin1(s_service),
                                                          path,
                                                          QString::fromLatin1(s_propertiesInterface),
                                                          QStringLiteral("Get"));
    message << interface << name;
    const QDBusReply<QDBusVariant> reply = QDBusConnection::sessionBus().call(message);
    return reply.isValid() ? reply.value().variant() : QVariant();
}

bool KWinInputBackend::writeProperty(const QString &path, const QString &name,
                                     const QVariant &value) const
{
    QDBusMessage message = QDBusMessage::createMethodCall(QString::fromLatin1(s_service),
                                                          path,
                                                          QString::fromLatin1(s_propertiesInterface),
                                                          QStringLiteral("Set"));
    message << QString::fromLatin1(s_deviceInterface) << name
            << QVariant::fromValue(QDBusVariant(value));
    const QDBusMessage reply = QDBusConnection::sessionBus().call(message);
    return reply.type() != QDBusMessage::ErrorMessage;
}

QString KWinInputBackend::devicePath(const QString &sysName)
{
    return QString::fromLatin1(s_managerPath) + QLatin1Char('/') + sysName;
}
