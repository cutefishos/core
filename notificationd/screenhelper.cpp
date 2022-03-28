/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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

#include "screenhelper.h"
#include <QApplication>
#include <QScreen>

ScreenHelper::ScreenHelper(QObject *parent)
    : QObject(parent)
{
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &ScreenHelper::screenGeometryChanged);
    connect(qGuiApp, &QGuiApplication::primaryScreenChanged, this, &ScreenHelper::availableScreenRectChanged);

    connect(qApp->primaryScreen(), &QScreen::geometryChanged, this, &ScreenHelper::screenGeometryChanged);
    connect(qApp->primaryScreen(), &QScreen::availableGeometryChanged, this, &ScreenHelper::availableScreenRectChanged);
}

QRect ScreenHelper::screenGeometry() const
{
    return qApp->primaryScreen()->geometry();
}

QRect ScreenHelper::availableScreenRect() const
{
    return qApp->primaryScreen()->availableGeometry();
}
