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

#ifndef SCREENHELPER_H
#define SCREENHELPER_H

#include <QObject>
#include <QRect>
#include <QScreen>

class ScreenHelper : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)
    Q_PROPERTY(QRect availableScreenRect READ availableScreenRect NOTIFY availableScreenRectChanged)

public:
    explicit ScreenHelper(QObject *parent = nullptr);

    QRect screenGeometry() const;
    QRect availableScreenRect() const;

signals:
    void screenGeometryChanged();
    void availableScreenRectChanged();
};

#endif // SCREENHELPER_H
