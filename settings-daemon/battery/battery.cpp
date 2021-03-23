#include "battery.h"
#include "power.h"
#include "primarybatteryadaptor.h"

#include <QDateTime>
#include <QDebug>

enum TimeConstants {
    SecsInDay = 86400,
    SecsInHour = 3600,
    SecsInMinute = 60
};

// ref: https://upower.freedesktop.org/docs/Device.html
//      kf5solid

Battery::Battery(UPowerDevice *device, QObject *parent)
    : QObject(parent)
    , m_device(device)
    , m_settings(nullptr)
    , m_lastChargedPercent(0)
    , m_lastChargedSecs(0)
{
    connect(device, SIGNAL(changed()), this, SLOT(slotChanged()));

    updateCache();
    init();
}

void Battery::updateCache()
{    
    m_chargeState = chargeState();
    m_isPresent = isPresent();
    m_chargePercent = chargePercent();
    m_capacity = capacity();
    m_isPowerSupply = isPowerSupply();
    m_timeToEmpty = timeToEmpty();
    m_timeToFull = timeToFull();
    m_energy = energy();
    m_energyFull = energyFull();
    m_energyFullDesign = energyFullDesign();
    m_energyRate = energyRate();
    m_voltage = voltage();
    m_temperature = temperature();
}

void Battery::init()
{
    if (type() == Battery::PrimaryBattery) {
        new PrimaryBatteryAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/PrimaryBattery"), this);

        m_settings = new QSettings(QStringLiteral("cutefishos"), QStringLiteral("PrimaryBattery"));
        m_lastChargedPercent = m_settings->value("LastChargedPercent", 0).toInt();
        m_lastChargedSecs = m_settings->value("LastChargedSecs").toLongLong();
    }
}

void Battery::refresh()
{
    QDBusInterface iface(UP_DBUS_SERVICE, m_device->udi(),
                         UP_DBUS_INTERFACE_DEVICE, QDBusConnection::systemBus());
    if (iface.isValid())
        iface.call("Refresh");
}

bool Battery::isPresent() const
{
    return m_device->prop("IsPresent").toBool();
}

Battery::BatteryType Battery::type() const
{
    Battery::BatteryType result = Battery::UnknownBattery;
    const uint t = m_device->prop("Type").toUInt();

    switch (t) {
        case UP_DEVICE_KIND_LINE_POWER: // TODO
            break;
        case UP_DEVICE_KIND_BATTERY:
            result = Battery::PrimaryBattery;
            break;
        case UP_DEVICE_KIND_UPS:
            result = Battery::UpsBattery;
            break;
        case UP_DEVICE_KIND_MONITOR:
            result = Battery::MonitorBattery;
            break;
        case UP_DEVICE_KIND_MOUSE:
            result = Battery::MouseBattery;
            break;
        case UP_DEVICE_KIND_KEYBOARD:
            result = Battery::KeyboardBattery;
            break;
        case UP_DEVICE_KIND_PDA:
            result = Battery::PdaBattery;
            break;
        case UP_DEVICE_KIND_PHONE:
            result = Battery::PhoneBattery;
            break;
        case UP_DEVICE_KIND_GAMING_INPUT:
            result = Battery::GamingInputBattery;
            break;
        case UP_DEVICE_KIND_UNKNOWN: {
            // There is currently no "Bluetooth battery" type, so check if it comes from Bluez
            if (m_device->prop("NativePath").toString().startsWith(QLatin1String("/org/bluez/"))) {
                result = Battery::BluetoothBattery;
            }
            break;
        }
    }

    return result;
}

int Battery::chargePercent() const
{
    return qRound(m_device->prop("Percentage").toDouble());
}

int Battery::capacity() const
{
    return m_device->prop("Capacity").toDouble();
}

bool Battery::isRechargeable() const
{
    return m_device->prop("IsRechargeable").toBool();
}

bool Battery::isPowerSupply() const
{
    return m_device->prop("PowerSupply").toBool();
}

Battery::ChargeState Battery::chargeState() const
{
    Battery::ChargeState result = Battery::NoCharge;
    const uint state = m_device->prop("State").toUInt();
    switch (state) {
    case 0:
        result = Battery::NoCharge; // stable or unknown
        break;
    case 1:
        result = Battery::Charging;
        break;
    case 2:
        result = Battery::Discharging;
        break;
    case 3: // TODO "Empty"
        break;
    case 4:
        result = Battery::FullyCharged;
        break;
    case 5: // TODO "Pending charge"
        break;
    case 6: // TODO "Pending discharge"
        break;
    }

    return result;
}

qlonglong Battery::timeToEmpty() const
{
    return m_device->prop("TimeToEmpty").toLongLong();
}

qlonglong Battery::timeToFull() const
{
    return m_device->prop("TimeToFull").toLongLong();
}

Battery::Technology Battery::technology() const
{
    const uint tech = m_device->prop("Technology").toUInt();
    switch (tech) {
    case 1:
        return Battery::LithiumIon;
    case 2:
        return Battery::LithiumPolymer;
    case 3:
        return Battery::LithiumIronPhosphate;
    case 4:
        return Battery::LeadAcid;
    case 5:
        return Battery::NickelCadmium;
    case 6:
        return Battery::NickelMetalHydride;
    default:
        return Battery::UnknownTechnology;
    }
}

double Battery::energy() const
{
    return m_device->prop("Energy").toDouble();
}

double Battery::energyFull() const
{
    return m_device->prop("EnergyFull").toDouble();
}

double Battery::energyFullDesign() const
{
    return m_device->prop("EnergyFullDesign").toDouble();
}

double Battery::energyRate() const
{
    return m_device->prop("EnergyRate").toDouble();
}

double Battery::voltage() const
{
    return m_device->prop("Voltage").toDouble();
}

double Battery::temperature() const
{
    return m_device->prop("Temperature").toDouble();
}

bool Battery::isRecalled() const
{
    return m_device->prop("RecallNotice").toBool();
}

QString Battery::recallVendor() const
{
    return m_device->prop("RecallVendor").toString();
}

QString Battery::recallUrl() const
{
    return m_device->prop("RecallUrl").toString();
}

QString Battery::serial() const
{
    return m_device->prop("Serial").toString();
}

qlonglong Battery::remainingTime() const
{
    if (chargeState() == Battery::Charging) {
        return timeToFull();
    } else if (chargeState() == Battery::Discharging) {
        return timeToEmpty();
    }

    return -1;
}

QString Battery::formatDuration(qlonglong seconds) const
{
    QString result;
    quint64 secs = seconds;

    int days = secs / SecsInDay;
    secs = secs % SecsInDay;
    int hours = secs / SecsInHour;
    secs = secs % SecsInHour;
    int minutes = secs / SecsInMinute;

    if (days > 0)
        result.push_back(tr("%1d").arg(days));

    if (hours > 0) {
        if (days > 0)
            result.push_back(" ");

        result.push_back(tr("%1h").arg(hours));
    }

    if (minutes > 0) {
        if (hours > 0)
            result.push_back(" ");

        result.push_back(tr("%1m").arg(minutes));
    }

    return result;
}

QString Battery::statusString() const
{
    if (chargeState() == Battery::Charging) {
        return tr("%1 until fully charged").arg(formatDuration(remainingTime()));
    } else if (chargeState() == Battery::Discharging) {
        return tr("%1 remaining").arg(formatDuration(remainingTime()));
    } else if (chargeState() == Battery::FullyCharged) {
        return tr("Fully charged.");
    }

    return QString();
}

int Battery::lastChargedPercent() const
{
    return m_lastChargedPercent;
}

quint64 Battery::lastChargedSecs() const
{
    return m_lastChargedSecs;
}

QString Battery::lastChargedTime() const
{
    QDateTime now = QDateTime::currentDateTime();
    QDateTime time = QDateTime::fromSecsSinceEpoch(m_lastChargedSecs);
    qint64 minutes = qRound64(time.secsTo(now) / 60.0f);

    if (minutes == 0)
        return tr("now");

    return tr("%1 ago").arg(formatDuration(time.secsTo(now)));
}

void Battery::slotChanged()
{
    if (m_device) {
        const bool old_isPresent = m_isPresent;
        const int old_chargePercent = m_chargePercent;
        const int old_capacity = m_capacity;
        const bool old_isPowerSupply = m_isPowerSupply;
        const Battery::ChargeState old_chargeState = m_chargeState;
        const qlonglong old_timeToEmpty = m_timeToEmpty;
        const qlonglong old_timeToFull = m_timeToFull;
        const double old_energy = m_energy;
        const double old_energyFull = m_energyFull;
        const double old_energyFullDesign = m_energyFullDesign;
        const double old_energyRate = m_energyRate;
        const double old_voltage = m_voltage;
        const double old_temperature = m_temperature;

        updateCache();

        if (old_isPresent != m_isPresent) {
            emit presentStateChanged(m_isPresent);
        }

        if (old_chargePercent != m_chargePercent) {
            emit chargePercentChanged(m_chargePercent);
        }

        if (old_capacity != m_capacity) {
            emit capacityChanged(m_capacity);
        }

        if (old_isPowerSupply != m_isPowerSupply) {
            emit powerSupplyStateChanged(m_isPowerSupply);
        }

        if (old_chargeState != m_chargeState) {
            emit chargeStateChanged(m_chargeState);
        }

        // Save last charge percentage
        if ((old_chargeState == Battery::Charging || old_chargeState == Battery::FullyCharged) && 
             m_chargeState == Battery::Discharging) {
            m_lastChargedPercent = m_chargePercent;
            m_lastChargedSecs = QDateTime::currentSecsSinceEpoch();

            if (m_settings) {
                m_settings->setValue("LastChargedPercent", m_lastChargedPercent);
                m_settings->setValue("LastChargedSecs", m_lastChargedSecs);
            }

            emit lastChargedPercentChanged();
            emit lastChargedSecsChanged();
        }

        if (old_timeToEmpty != m_timeToEmpty) {
            emit timeToEmptyChanged(m_timeToEmpty);
        }

        if (old_timeToFull != m_timeToFull) {
            emit timeToFullChanged(m_timeToFull);
        }

        if (old_energy != m_energy) {
            emit energyChanged(m_energy);
        }

        if (old_energyFull != m_energyFull) {
            emit energyFullChanged(m_energyFull);
        }

        if (old_energyFullDesign != m_energyFullDesign) {
            emit energyFullChanged(m_energyFullDesign);
        }

        if (old_energyRate != m_energyRate) {
            emit energyRateChanged(m_energyRate);
        }

        if (old_voltage != m_voltage) {
            emit voltageChanged(m_voltage);
        }

        if (old_temperature != m_temperature) {
            emit temperatureChanged(m_temperature);
        }

        if (old_timeToFull != m_timeToFull || old_timeToEmpty != m_timeToEmpty) {
            emit remainingTimeChanged(remainingTime());
        }
    }
}
