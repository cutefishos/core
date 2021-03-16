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

#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDir>
#include <QList>
#include <QFile>

inline QString readFile(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QString data = QString::fromUtf8(file.readAll());
        file.close();
        return data;
    }

    return QString();
}

inline bool writeFile(const QString &fileName, const QString &data)
{
    QFile file(fileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        textStream << data;
        textStream.flush();
        file.close();
        return true;
    }

    return false;
}

class ScreenDevice
{
public:
    void set(int value) {
        if (value <= 0)
            value = 1;
        else if (value > 100)
            value = 100;

        const int brightness = static_cast<int>(value * maxValue * 0.01);
        QString brightnessData = QString::number(brightness, 10);
        const QString brightnessFile = QDir(name).filePath("brightness");
        writeFile(brightnessFile, brightnessData);
    }

    int actual;
    int maxValue;
    QString name;
};

inline QList<ScreenDevice> getDeviceList()
{
    const char kSysBrightnessDir[] = "/sys/class/backlight";
    const char kMaxFile[] = "max_brightness";
    const char kActualFile[] = "actual_brightness";
    QList<ScreenDevice> list;
    QDir sys(kSysBrightnessDir);

    for (const QFileInfo &device : sys.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        const QString dev_path = device.filePath();
        QDir dev(dev_path);
        const QString max_brightness = readFile(dev.filePath(kMaxFile));
        bool max_ok;
        const int max = max_brightness.toInt(&max_ok, 10);

        if (!max_ok) {
            qWarning() << "Failed to read max brightness:" << dev_path;
            continue;
        }

        const QString actual_brightness = readFile(dev.filePath(kActualFile));
        bool actual_ok;
        const int actual = actual_brightness.toInt(&actual_ok, 10);
        if (!actual_ok) {
            qWarning() << "Failed to read actual brightness:" << dev_path;
            continue;
        }

        ScreenDevice screen;
        screen.actual = actual;
        screen.maxValue = max;
        screen.name = dev_path;
        list << screen;
    }

    return list;
}

inline void setBrightness(int value)
{
    for (ScreenDevice &device : getDeviceList()) {
        device.set(value);
    }
}

inline void increaseBrightness()
{
    for (ScreenDevice &device : getDeviceList()) {
        int value = static_cast<int>(device.actual * 100.0 / device.maxValue);
        device.set(value + 20);
    }
}

inline void decreaseBrightness()
{
    for (ScreenDevice &device : getDeviceList()) {
        int value = static_cast<int>(device.actual * 100.0 / device.maxValue);
        device.set(value - 20);
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption setOption(QStringLiteral("set"), QStringLiteral("Set screen brightness"));
    parser.addOption(setOption);
    parser.addPositionalArgument("value", "Value");

    QCommandLineOption increaseOption(QStringLiteral("i"), QStringLiteral("Increase brightness"));
    parser.addOption(increaseOption);

    QCommandLineOption decreaseOption(QStringLiteral("d"), QStringLiteral("Decrease brightness"));
    parser.addOption(decreaseOption);

    parser.process(app);

    if (parser.isSet(setOption)) {
        int value = parser.positionalArguments().value(0).toInt();
        setBrightness(value);
    } else if (parser.isSet(increaseOption)){
        increaseBrightness();
    } else if (parser.isSet(decreaseOption)) {
        decreaseBrightness();
    }

    return 0;
}
