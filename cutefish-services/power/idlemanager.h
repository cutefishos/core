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

#ifndef IDLEMANAGER_H
#define IDLEMANAGER_H

#include <QObject>
#include <QHash>
#include <QSet>

#include "action.h"

class IdleManager : public QObject
{
    Q_OBJECT

public:
    static IdleManager *self();
    explicit IdleManager(QObject *parent = nullptr);

    void registerActionTimeout(Action *action, int secs);
    void unregisterActionTimeouts(Action *action);

private slots:
    void onKIdleTimeoutReached(int, int);
    void onResumingFromIdle();

private:
    friend class Action;

    QHash<Action *, QList<int>> m_registeredActionTimeouts;
    QSet<Action *> m_pendingResumeFromIdleActions;
};

#endif // IDLEMANAGER_H
