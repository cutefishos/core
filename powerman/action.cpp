/***************************************************************************
 *   Copyright (C) 2010 by Dario Freddi <drf@kde.org>                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "action.h"
#include "idlemanager.h"

#include <QDebug>

class Action::Private
{
public:
    Private() {}
    ~Private() {}

    QVector< int > registeredIdleTimeouts;
};

Action::Action(QObject* parent)
        : QObject(parent)
        , d(new Private)
{
}

Action::~Action()
{
    delete d;
}

void Action::registerIdleTimeout(int msec)
{
    d->registeredIdleTimeouts.append(msec);
    IdleManager::self()->registerActionTimeout(this, msec);
}

bool Action::isSupported()
{
    return true;
}

void Action::setTimeout(int timeout)
{
    Q_UNUSED(timeout)
}

