/***************************************************************************
 *   Copyright (C) 2021 by Reion Wong <reion@cutefishos.com>               *
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

#ifndef DIMDISPLAYACTION_H
#define DIMDISPLAYACTION_H

#include "action.h"
#include <QDBusInterface>

class DimDisplayAction : public Action
{
    Q_OBJECT

public:
    explicit DimDisplayAction(QObject *parent = nullptr);

    void onWakeupFromIdle() override;
    void onIdleTimeout(int msec) override;
    void setTimeout(int timeout) override;
    void setSleep(bool sleep);
    void setLock(bool lock);

private:
    QDBusInterface m_iface;
    int m_dimOnIdleTime = 0;
    int m_oldScreenBrightness = 0;
    bool m_dimmed = false;
    bool m_sleep = false;
    bool m_lock = false;
};

#endif // DIMDISPLAYACTION_H
