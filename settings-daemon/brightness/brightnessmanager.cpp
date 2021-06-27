/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "brightnessmanager.h"
#include "brightnessadaptor.h"
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QDir>

#include <sys/utsname.h>

#define PREFIX "/sys/class/backlight/"

BrightnessManager::BrightnessManager(QObject *parent)
    : QObject(parent),
      m_fileWatcher(new QFileSystemWatcher(this))
{
    // init dbus
    new BrightnessAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Brightness"), this);

    init();
}

BrightnessManager::~BrightnessManager()
{
}

int BrightnessManager::maxBrightness()
{
    int max_brightness;
    QFile file(m_dirname + "/max_brightness");
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "reading max brightness failed with error code " << file.error() << file.errorString();
        return -1; // some non-zero value
    }

    QTextStream stream(&file);
    stream >> max_brightness;
    file.close();

    return max_brightness ? max_brightness : -1;
}

int BrightnessManager::brightness()
{
    QFile file(m_dirname + "/brightness");
    if (!file.open(QIODevice::ReadOnly)) {
        return -1;
    }

    int brightness;
    QTextStream stream(&file);
    stream >> brightness;
    file.close();

    return brightness * 100 / maxBrightness();
}

void BrightnessManager::setValue(int value)
{
    QProcess process;
    process.start("pkexec", QStringList() << "cutefish-screen-brightness" << "--set" << QString::number(value));
    process.waitForFinished(-1);
}

void BrightnessManager::init()
{
    if (useWhitelistInit())
        initUsingWhitelist();
    else
        initUsingBacklightType();

    if (!m_dirname.isEmpty()) {
        m_fileWatcher->addPath(m_dirname + "/actual_brightness");
        m_fileWatcher->addPath(m_dirname + "/brightness");
        m_fileWatcher->addPath(m_dirname + "/bl_power");
        m_actualBacklight = brightness();
        connect(m_fileWatcher, &QFileSystemWatcher::fileChanged, this, &BrightnessManager::handleFileChanged);
    }
}

bool BrightnessManager::useWhitelistInit()
{
    struct utsname uts;
    uname(&uts);

    int major, minor, patch, result;
    result = sscanf(uts.release, "%d.%d", &major, &minor);

    if (result != 2) {
        return true; // Malformed version
    }

    if (major == 3) {
        return false; //Kernel 3, we want type based init
    }

    result = sscanf(uts.release, "%d.%d.%d", &major, &minor, &patch);

    if (result != 3) {
        return true; // Malformed version
    }

    if (patch < 37) {
        return true; //Minor than 2.6.37, use whiteList based
    }

    return false;//Use Type based interface
}

void BrightnessManager::initUsingWhitelist()
{
    QStringList interfaces;
    interfaces << "nv_backlight" << "radeon_bl" << "mbp_backlight" << "asus_laptop"
               << "toshiba" << "eeepc" << "thinkpad_screen" << "acpi_video1" << "acpi_video0"
               << "intel_backlight" << "apple_backlight" << "fujitsu-laptop" << "samsung"
               << "nvidia_backlight" << "dell_backlight" << "sony" << "pwm-backlight";

    QDir dir;
    foreach (const QString & interface, interfaces) {
        dir.setPath(PREFIX + interface);
        //qDebug() << "searching dir:" << dir;
        if (dir.exists()) {
            m_dirname = dir.path();
            //qDebug() << "kernel backlight support found in" << m_dirname;
            break;
        }
    }

    //If none of our whitelisted interface is available, get the first one  (if any)
    if (m_dirname.isEmpty()) {
        dir.setPath(PREFIX);
        dir.setFilter(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot | QDir::NoDotAndDotDot | QDir::Readable);
        QStringList dirList = dir.entryList();
        if (!dirList.isEmpty()) {
            m_dirname = PREFIX + dirList.first();
        }
    }
}

void BrightnessManager::initUsingBacklightType()
{
    QString m_dirname;
    QDir dir(PREFIX);
    dir.setFilter(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot | QDir::NoDotAndDotDot | QDir::Readable);
    dir.setSorting(QDir::Name | QDir::Reversed);// Reverse is needed to priorize acpi_video1 over 0

    QStringList interfaces = dir.entryList();

    if (interfaces.isEmpty()) {
        return;
    }

    QFile file;
    QByteArray buffer;
    QStringList firmware, platform, raw;

    foreach(const QString & interface, interfaces) {
        file.setFileName(PREFIX + interface + "/type");
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }

        buffer = file.readLine().trimmed();
        if (buffer == "firmware") {
            firmware.append(interface);
        } else if(buffer == "platform") {
            platform.append(interface);
        } else if (buffer == "raw") {
            raw.append(interface);
        } else {
            qWarning() << "Interface type not handled" << buffer;
        }

        file.close();
    }

    if (!firmware.isEmpty()) {
        m_dirname = PREFIX + firmware.first();
        return;
    }

    if (!platform.isEmpty()) {
        m_dirname = PREFIX + platform.first();
        return;
    }

    if (!raw.isEmpty()) {
        m_dirname = PREFIX + raw.first();
        return;
    }
}

void BrightnessManager::handleFileChanged(const QString &path)
{
    Q_UNUSED(path);

    int newValue = brightness();

    if (m_actualBacklight != newValue) {
        m_actualBacklight = newValue;
        Q_EMIT brightnessChanged();
    }
}

bool BrightnessManager::brightnessEnabled()
{
    QDir dir;
    dir.setPath(PREFIX);
    dir.setFilter(QDir::AllDirs | QDir::NoDot | QDir::NoDotDot | QDir::NoDotAndDotDot | QDir::Readable);
    QStringList dirList = dir.entryList();
    return !dirList.isEmpty();
}
