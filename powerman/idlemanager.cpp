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

#include "idlemanager.h"

#include <KIdleTime>

IdleManager *IdleManager::self()
{
    static IdleManager mgr;
    return &mgr;
}

IdleManager::IdleManager(QObject *parent)
    : QObject(parent)
{
    connect(KIdleTime::instance(), SIGNAL(timeoutReached(int,int)),
            this, SLOT(onKIdleTimeoutReached(int,int)));
    connect(KIdleTime::instance(), &KIdleTime::resumingFromIdle,
            this, &IdleManager::onResumingFromIdle);
}

void IdleManager::registerActionTimeout(Action *action, int secs)
{
    int identifier = KIdleTime::instance()->addIdleTimeout(secs * 1000);

    QList<int> timeouts = m_registeredActionTimeouts[action];
    timeouts.append(identifier);

    m_registeredActionTimeouts[action] = timeouts;
}

void IdleManager::unregisterActionTimeouts(Action *action)
{
    // Clear all timeouts from the action
    const QList< int > timeoutsToClean = m_registeredActionTimeouts[action];

    for (int id : timeoutsToClean) {
        KIdleTime::instance()->removeIdleTimeout(id);
    }

    m_registeredActionTimeouts.remove(action);
}

void IdleManager::onKIdleTimeoutReached(int identifier, int timeout)
{
    for (auto i = m_registeredActionTimeouts.constBegin(), end = m_registeredActionTimeouts.constEnd(); i != end; ++i) {
        if (i.value().contains(identifier)) {
            i.key()->onIdleTimeout(timeout);

            // And it will need to be awaken
            m_pendingResumeFromIdleActions.insert(i.key());

            break;
        }
    }

    // Catch the next resume event if some actions require it
    if (!m_pendingResumeFromIdleActions.isEmpty()) {
        KIdleTime::instance()->catchNextResumeEvent();
    }
}

void IdleManager::onResumingFromIdle()
{
    KIdleTime::instance()->simulateUserActivity();
    // Wake up the actions in which an idle action was triggered
    std::for_each(m_pendingResumeFromIdleActions.cbegin(), m_pendingResumeFromIdleActions.cend(),
        std::mem_fn(&Action::onWakeupFromIdle));

    m_pendingResumeFromIdleActions.clear();
}
