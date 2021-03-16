#ifndef BATTERY_H
#define BATTERY_H

#include <QObject>
#include <QTimer>
#include <QSettings>
#include "upowerdevice.h"

class Battery : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int isPresent READ isPresent)
    Q_PROPERTY(int type READ type)
    Q_PROPERTY(int chargePercent READ chargePercent NOTIFY chargePercentChanged)
    Q_PROPERTY(int capacity READ capacity NOTIFY capacityChanged)
    Q_PROPERTY(int voltage READ voltage NOTIFY voltageChanged)
    Q_PROPERTY(int isRechargeable READ isRechargeable)
    Q_PROPERTY(int isPowerSupply READ isPowerSupply)
    Q_PROPERTY(int chargeState READ chargeState NOTIFY chargeStateChanged)
    Q_PROPERTY(qlonglong remainingTime READ remainingTime NOTIFY remainingTimeChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY remainingTimeChanged)
    Q_PROPERTY(int lastChargedPercent READ lastChargedPercent NOTIFY lastChargedPercentChanged)
    Q_PROPERTY(quint64 lastChargedSecs READ lastChargedSecs NOTIFY lastChargedSecsChanged)
    Q_PROPERTY(QString lastChargedTime READ lastChargedTime NOTIFY lastChargedSecsChanged)
    Q_PROPERTY(QString udi READ udi)

public:
    /**
     * This enum type defines the type of the device holding the battery
     *
     * - PdaBattery : A battery in a Personal Digital Assistant
     * - UpsBattery : A battery in an Uninterruptible Power Supply
     * - PrimaryBattery : A primary battery for the system (for example laptop battery)
     * - MouseBattery : A battery in a mouse
     * - KeyboardBattery : A battery in a keyboard
     * - KeyboardMouseBattery : A battery in a combined keyboard and mouse
     * - CameraBattery : A battery in a camera
     * - PhoneBattery : A battery in a phone
     * - MonitorBattery : A battery in a monitor
     * - GamingInputBattery : A battery in a gaming input device (for example a wireless game pad)
     * - BluetoothBattery: A generic Bluetooth device battery (if its type isn't known, a Bluetooth
     *                     mouse would normally show up as a MouseBattery), @since 5.54
     * - UnknownBattery : A battery in an unknown device
     */
    enum BatteryType { UnknownBattery, PdaBattery, UpsBattery,
                       PrimaryBattery, MouseBattery, KeyboardBattery,
                       KeyboardMouseBattery, CameraBattery,
                       PhoneBattery, MonitorBattery, GamingInputBattery,
                       BluetoothBattery
                     };
    Q_ENUM(BatteryType)

    /**
     * This enum type defines charge state of a battery
     *
     * - NoCharge : Battery charge is stable, not charging or discharging or
     *              the state is Unknown
     * - Charging : Battery is charging
     * - Discharging : Battery is discharging
     * - FullyCharged: The battery is fully charged; a battery not necessarily
     *                 charges up to 100%
     */
    enum ChargeState { NoCharge, Charging, Discharging, FullyCharged };
    Q_ENUM(ChargeState)

    /**
      * Technology used in the battery
      *
      * 0: Unknown
      * 1: Lithium ion
      * 2: Lithium polymer
      * 3: Lithium iron phosphate
      * 4: Lead acid
      * 5: Nickel cadmium
      * 6: Nickel metal hydride
      */
    enum Technology { UnknownTechnology = 0, LithiumIon, LithiumPolymer, LithiumIronPhosphate,
                      LeadAcid, NickelCadmium, NickelMetalHydride
                    };
    Q_ENUM(Technology)

    // Begin

    explicit Battery(UPowerDevice *device, QObject *parent = nullptr);

    void updateCache();
    void init();
    void refresh();

    bool isPresent() const;
    Battery::BatteryType type() const;

    int chargePercent() const;
    int capacity() const;

    bool isRechargeable() const;

    bool isPowerSupply() const;

    Battery::ChargeState chargeState() const;

    // Number of seconds until the power source is considered empty. Is set to 0 if unknown.
    // This property is only valid if the property type has the value "battery".
    qlonglong timeToEmpty() const;
    // Number of seconds until the power source is considered full. Is set to 0 if unknown.
    // This property is only valid if the property type has the value "battery".
    qlonglong timeToFull() const;

    Battery::Technology technology() const;

    double energy() const;

    double energyFull() const;

    double energyFullDesign() const;

    double energyRate() const;

    double voltage() const;

    double temperature() const;

    bool isRecalled() const;

    QString recallVendor() const;

    QString recallUrl() const;

    QString serial() const;

    qlonglong remainingTime() const;

    QString formatDuration(qlonglong seconds) const;
    QString statusString() const;
    int lastChargedPercent() const;
    quint64 lastChargedSecs() const;

    QString lastChargedTime() const;

    QString udi() const { return m_device->udi(); }

signals:
    void presentStateChanged(bool newState);
    void chargePercentChanged(int value);
    void capacityChanged(int value);
    void powerSupplyStateChanged(bool newState);
    void chargeStateChanged(int newState);
    void timeToEmptyChanged(qlonglong time);
    void timeToFullChanged(qlonglong time);
    void energyChanged(double energy);
    void energyFullChanged(double energyFull);
    void energyFullDesignChanged(double energyFullDesign);
    void energyRateChanged(double energyRate);
    void voltageChanged(double voltage);
    void temperatureChanged(double temperature);
    void remainingTimeChanged(qlonglong time);
    void lastChargedPercentChanged();
    void lastChargedSecsChanged();

private slots:
    void slotChanged();

private:
    UPowerDevice *m_device;

    bool m_isPresent;
    int m_chargePercent;
    int m_capacity;
    bool m_isPowerSupply;
    Battery::ChargeState m_chargeState;
    qlonglong m_timeToEmpty;
    qlonglong m_timeToFull;
    double m_energy;
    double m_energyFull;
    double m_energyFullDesign;
    double m_energyRate;
    double m_voltage;
    double m_temperature;

    QSettings *m_settings;
    int m_lastChargedPercent;
    quint64 m_lastChargedSecs;

    // QTimer m_refreshTimer;
};

#endif // BATTERY_H
