#ifndef CUTEFISH_SETTINGS_POWERMANAGER_H
#define CUTEFISH_SETTINGS_POWERMANAGER_H

#include <QDBusInterface>
#include <QObject>
#include <QSettings>

class DimDisplayAction;
class UPowerManager;

class PowerManager : public QObject
{
    Q_OBJECT

public:
    explicit PowerManager(UPowerManager *upowerManager, QObject *parent = nullptr);

public slots:
    void setBatteryScreenOff(int timeout);
    void setACScreenOff(int timeout);
    void setDimDisplayTimeout(int timeout);
    void setSleepWhenClosedScreen(bool enabled);
    void setLockWhenClosedScreen(bool enabled);

private slots:
    void onBatteryChanged();

private:
    void loadSettings();
    void applyPolicy();
    void writePowerSetting(const QString &key, const QVariant &value);

    QSettings m_settings;
    UPowerManager *m_upowerManager;
    DimDisplayAction *m_dimDisplayAction;

    int m_batteryScreenOff = 300;
    int m_acScreenOff = 1200;
    bool m_sleepWhenClosedScreen = false;
    bool m_lockWhenClosedScreen = true;
    bool m_onBattery = false;
};

#endif
