/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     pjx <pjx206@163.com>
 *             rekols <rekols@foxmail.com>
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

#include "brightnesshelper.h"

BrightnessHelper::BrightnessHelper(QObject *parent)
    : QObject(parent)
{
    init();
    anime.setEasingCurve(QEasingCurve::Linear);
    connect(&anime, &QVariantAnimation::valueChanged, this, 
        [this](const QVariant &value){
        if (QAbstractAnimation::Running == anime.state()) {
            writeBrightness(value.toInt());
        }
    });
    connect(&anime, &QVariantAnimation::finished, this, [this]() {
        qApp->quit();
    });
}

BrightnessHelper::~BrightnessHelper()
{
}

void BrightnessHelper::init()
{
    const char kSysBrightnessDir[] = "/sys/class/backlight";
    const char kMaxFile[] = "max_brightness";
    const char kActualFile[] = "actual_brightness";
    QDir sys(kSysBrightnessDir);

    QStringList firmware, platform, raw;

    QFile file;
    QByteArray typeBuffer;
    for (const QFileInfo &device: sys.entryInfoList(
        QDir::Dirs | QDir::NoDotAndDotDot | QDir::Readable, 
        QDir::Name | QDir::Reversed)) {
        const QString dev_path = device.filePath();
        file.setFileName(dev_path + "/type");
        
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            continue;
        }
        
        typeBuffer = file.readLine().trimmed();

        if (typeBuffer == "firmware") {
            firmware.append(dev_path);
        } else if(typeBuffer == "platform") {
            platform.append(dev_path);
        } else if (typeBuffer == "raw") {
            raw.append(dev_path);
        } else {
            qWarning() << "unsupported type in '"
                       << file.fileName() << "' : " << QString(typeBuffer);
        }

        file.close();
    }

    if (!firmware.isEmpty())
        name = firmware.constFirst();
    else if (!platform.isEmpty())
        name = platform.constFirst();
    else if (!raw.isEmpty())
        name = raw.constFirst();
    
    actual = readFile(QDir(name).filePath(kActualFile)).toInt();
    maxValue = readFile(QDir(name).filePath(kMaxFile)).toInt();
}

void BrightnessHelper::setBrightness(int value)
{
    if (value < 0)
        value = 0;
    if (value > 100)
        value = 100;
    anime.setDuration(85);
    anime.setStartValue(actual);
    const int end = static_cast<int>(value /100.0 * maxValue);
    anime.setEndValue(end);
    anime.start();
}

void BrightnessHelper::increaseBrightness()
{
    int value = static_cast<int>(actual / (double)maxValue * 100);
    setBrightness(value + 20);
}

void BrightnessHelper::decreaseBrightness()
{
    int value = static_cast<int>(actual / (double)maxValue * 100);
    setBrightness(value - 20);
}

void BrightnessHelper::writeBrightness(int brightness)
{
    if (brightness < 0)
        brightness = 0;
    if (brightness > maxValue)
        brightness = maxValue;
    QString dbg_s = QDir(name).filePath("brightness");
    writeFile(QDir(name).filePath("brightness"), QString::number(brightness));
}
