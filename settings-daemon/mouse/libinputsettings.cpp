/*
    SPDX-FileCopyrightText: 2018 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "libinputsettings.h"

#include <QDebug>
#include <QSettings>

template<>
bool LibinputSettings::load(QString key, bool defVal)
{
    QSettings settings("cutefishos", "mouse");
    return settings.value(key, defVal).toBool();
}

template<>
qreal LibinputSettings::load(QString key, qreal defVal)
{
    QSettings settings("cutefishos", "mouse");
    return settings.value(key, defVal).toReal();
}

template<>
void LibinputSettings::save(QString key, bool val)
{
    QSettings settings("cutefishos", "mouse");
    settings.setValue(key, val);
    settings.sync();
}

template<>
void LibinputSettings::save(QString key, qreal val)
{
    QSettings settings("cutefishos", "mouse");
    settings.setValue(key, val);
    settings.sync();
}
