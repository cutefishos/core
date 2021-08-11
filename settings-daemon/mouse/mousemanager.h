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

#ifndef MOUSE_H
#define MOUSE_H

#include <QObject>
#include "x11libinputdummydevice.h"

class Mouse : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool leftHanded READ leftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)
    Q_PROPERTY(bool acceleration READ acceleration WRITE setAcceleration NOTIFY accelerationChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration NOTIFY pointerAccelerationChanged)

public:
    explicit Mouse(QObject *parent = nullptr);
    ~Mouse();

    bool leftHanded() const;
    void setLeftHanded(bool enabled);

    bool acceleration() const;
    void setAcceleration(bool enabled);

    bool naturalScroll() const;
    void setNaturalScroll(bool enabled);

    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

signals:
    void leftHandedChanged();
    void accelerationChanged();
    void naturalScrollChanged();
    void pointerAccelerationChanged();

private:
    X11LibinputDummyDevice *m_inputDummydevice;
};

#endif // MOUSE_H
