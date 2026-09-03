#ifndef KWININPUTBACKEND_H
#define KWININPUTBACKEND_H

#include <QObject>
#include <QStringList>
#include <QVariant>

class QDBusServiceWatcher;

class KWinInputBackend : public QObject
{
    Q_OBJECT

public:
    enum class DeviceType {
        Pointer,
        Touchpad,
    };

    explicit KWinInputBackend(DeviceType type, QObject *parent = nullptr);

    bool available() const;
    bool booleanProperty(const QString &name, bool fallback = false) const;
    qreal realProperty(const QString &name, qreal fallback = 0.0) const;

    void setBooleanProperty(const QString &name, bool value);
    void setRealProperty(const QString &name, qreal value);

signals:
    void devicesChanged();

private slots:
    void refreshDevices();
    void handleDeviceAdded(const QString &sysName);
    void handleDeviceRemoved(const QString &sysName);

private:
    QStringList matchingDevicePaths() const;
    bool matches(const QString &path) const;
    QVariant readProperty(const QString &path, const QString &interface,
                          const QString &name) const;
    bool writeProperty(const QString &path, const QString &name,
                       const QVariant &value) const;
    static QString devicePath(const QString &sysName);

    DeviceType m_type;
    QStringList m_deviceNames;
    QDBusServiceWatcher *m_serviceWatcher;
};

#endif // KWININPUTBACKEND_H
