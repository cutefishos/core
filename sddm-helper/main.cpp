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

    QCommandLineOption scaleOption(QStringLiteral("scale"), QStringLiteral("Set greeter scale factor"),
                                   QStringLiteral("factor"));
    parser.addOption(scaleOption);

    parser.process(app);

    if (parser.isSet(scaleOption)) {
        bool ok = false;
        const qreal value = parser.value(scaleOption).toDouble(&ok);
        if (!ok || value < 1.0 || value > 4.0)
            return 1;

        QDir dir("/etc/sddm.conf.d");

        if (!dir.exists()) {
            dir.mkpath("/etc/sddm.conf.d");
        }

        QSettings settings("/etc/sddm.conf.d/scale.conf", QSettings::IniFormat);
        settings.beginGroup("General");
        settings.setValue("GreeterEnvironment",
                          QStringLiteral("QT_SCALE_FACTOR=%1").arg(value, 0, 'g', 3));
    }

    return 0;
}
