#ifndef CUTEFISH_SETTINGS_POWERMANAGER_H
#define CUTEFISH_SETTINGS_POWERMANAGER_H

#include <QObject>
#include <QSettings>

class DimDisplayAction;
class UPowerManager;
class CPUManagement;

class PowerManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int mode READ mode WRITE setMode NOTIFY modeChanged)

public:
    explicit PowerManager(UPowerManager *upowerManager,
                          CPUManagement *cpuManagement,
                          QObject *parent = nullptr);

    int mode() const;
    void setMode(int mode);

public slots:
    void setBatteryScreenOff(int timeout);
    void setACScreenOff(int timeout);
    void setDimDisplayTimeout(int timeout);

signals:
    void modeChanged();

private slots:
    void onBatteryChanged();
    void onLidClosedChanged();

private:
    void loadSettings();
    void applyPolicy();
    void writePowerSetting(const QString &key, const QVariant &value);

    QSettings m_settings;
    UPowerManager *m_upowerManager;
    CPUManagement *m_cpuManagement;
    DimDisplayAction *m_dimDisplayAction;

    int m_batteryScreenOff = 300;
    int m_acScreenOff = 1200;
    bool m_onBattery = false;
};

#endif
