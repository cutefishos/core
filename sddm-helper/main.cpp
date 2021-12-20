/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet (kate@cutefishos.com)
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

#include <QCommandLineParser>
#include <QFile>
#include <QDir>
#include <QSettings>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCommandLineParser parser;
    parser.addHelpOption();
    parser.addVersionOption();

    QCommandLineOption dpiOption(QStringLiteral("dpi"), QStringLiteral("Set DPI"));
    parser.addOption(dpiOption);
    parser.addPositionalArgument("value", "Value");

    parser.process(app);

    if (parser.isSet(dpiOption)) {
        int value = parser.positionalArguments().value(0).toInt();
        int minValue = 96;

        if (value < minValue)
            value = minValue;

        QDir dir("/etc/sddm.conf.d");

        if (!dir.exists()) {
            dir.mkpath("/etc/sddm.conf.d");
        }

        QSettings settings("/etc/sddm.conf.d/dpi.conf", QSettings::IniFormat);
        settings.beginGroup("X11");
        settings.setValue("ServerArguments", QString("-nolisten tcp -dpi %1").arg(value));
    }

    return 0;
}
