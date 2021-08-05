#ifndef LIDWATCHER_H
#define LIDWATCHER_H

#include <QObject>
#include <QDBusInterface>

class LidWatcher : public QObject
{
    Q_OBJECT

public:
    explicit LidWatcher(QObject *parent = nullptr);

    bool existLid() const;
    bool lidClosed() const;

signals:
    void lidClosedChanged();

private slots:
    void onPropertiesChanged();

private:
    QDBusInterface *m_uPowerIface;
    QDBusInterface *m_uPowerPropertiesIface;

    bool m_lidClosed;
};

#endif // LIDWATCHER_H
