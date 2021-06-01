/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
 *             pjx <pjx206@163.com>
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
#include "brightnesshelper.h"

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

    BrightnessHelper *brightnessHelper = new BrightnessHelper();

    if (parser.isSet(setOption))
    {
        int value = parser.positionalArguments().value(0).toInt();
        brightnessHelper->setBrightness(value);
    }
    else if (parser.isSet(increaseOption))
        brightnessHelper->increaseBrightness();
    else if (parser.isSet(decreaseOption))
        brightnessHelper->decreaseBrightness();

    app.exec();
}
