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

#include "mousemanager.h"
#include "mouseadaptor.h"

Mouse::Mouse(QObject *parent)
    : QObject(parent)
    , m_inputDummydevice(new X11LibinputDummyDevice(this, QX11Info::display()))
{
    // init dbus
    new MouseAdaptor(this);
    QDBusConnection::sessionBus().registerObject(QStringLiteral("/Mouse"), this);

    connect(m_inputDummydevice, &X11LibinputDummyDevice::leftHandedChanged, this, &Mouse::leftHandedChanged);
    connect(m_inputDummydevice, &X11LibinputDummyDevice::pointerAccelerationProfileChanged, this, &Mouse::accelerationChanged);
    connect(m_inputDummydevice, &X11LibinputDummyDevice::naturalScrollChanged, this, &Mouse::naturalScrollChanged);
    connect(m_inputDummydevice, &X11LibinputDummyDevice::pointerAccelerationChanged, this, &Mouse::pointerAccelerationChanged);
}

Mouse::~Mouse()
{
    delete m_inputDummydevice;
}

bool Mouse::leftHanded() const
{
    return m_inputDummydevice->isLeftHanded();
}

void Mouse::setLeftHanded(bool enabled)
{
    m_inputDummydevice->setLeftHanded(enabled);
    m_inputDummydevice->applyConfig();
}

bool Mouse::acceleration() const
{
    return m_inputDummydevice->pointerAccelerationProfileFlat();
}

void Mouse::setAcceleration(bool enabled)
{
    m_inputDummydevice->setPointerAccelerationProfileFlat(enabled);
    m_inputDummydevice->applyConfig();
}

bool Mouse::naturalScroll() const
{
    return m_inputDummydevice->isNaturalScroll();
}

void Mouse::setNaturalScroll(bool enabled)
{
    m_inputDummydevice->setNaturalScroll(enabled);
    m_inputDummydevice->applyConfig();
}

qreal Mouse::pointerAcceleration() const
{
    return m_inputDummydevice->pointerAcceleration();
}

void Mouse::setPointerAcceleration(qreal value)
{
    m_inputDummydevice->setPointerAcceleration(value);
    m_inputDummydevice->applyConfig();
}
