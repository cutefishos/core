/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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
#include <QProcess>
#include <QFile>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("Cutefish CPU frequency tool"));
    parser.addHelpOption();

    QCommandLineOption setOption(QStringList() << "s" << "set" << "Setting");
    parser.addOption(setOption);

    QCommandLineOption numberOption(QStringList() << "c" << "number" << "Number");
    parser.addOption(numberOption);

    QCommandLineOption modeOption(QStringList() << "m" << "mode" << "Mode");
    parser.addOption(modeOption);

    parser.process(a);

    // cutefish-cpufreq --set -n 0 -m performance
    if (parser.isSet(setOption) && parser.isSet(numberOption) && parser.isSet(modeOption)) {
        QString modeStr = parser.positionalArguments().last();

        if (modeStr != "powersave" && modeStr != "performance") {
            return -1;
        }

        QFile file(QString("/sys/devices/system/cpu/cpu%1/cpufreq/scaling_governor").arg(parser.positionalArguments().first()));

        if (!file.open(QIODevice::WriteOnly)) {
            return -1;
        }

        file.write(modeStr.toUtf8());
        file.close();

        // Set intel gpu.
        if (QFile("/usr/bin/intel_gpu_frequency").exists()) {
            QProcess process;
            process.setProgram("/usr/bin/intel_gpu_frequency");

            if (modeStr == "powersave") {
                process.setArguments(QStringList() << "-d");
            } else {
                process.setArguments(QStringList() << "-m");
            }
            process.start();
            process.waitForFinished();
        }
    }

    return 0;
}
