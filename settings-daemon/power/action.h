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


#ifndef POWERDEVIL_POWERDEVILACTION_H
#define POWERDEVIL_POWERDEVILACTION_H

#include <QObject>
#include <QVariantMap>

class IDleManager;
class Q_DECL_EXPORT Action : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Action)

public:
    /**
     * Default constructor
     */
    explicit Action(QObject *parent);
    /**
     * Default destructor
     */
    ~Action() override;

    /**
     * This function is meant to find out if this action is available on this system. Actions
     * CAN reimplement this function if they are dependent on specific hardware/software requirements.
     * By default, this function will always return true.
     *
     * Should this function return false, the core will delete and ignore the action right after creation.
     *
     * @returns Whether this action is supported or not by the current system
     */
    virtual bool isSupported();

    virtual void setTimeout(int timeout);

protected:
    /**
     * Registers an idle timeout for this action. Call this function and not KIdleTime directly to take advantage
     * of Action's automated handling of idle timeouts. Also, please reimplement onIdleTimeout instead of listening
     * to KIdleTime's signals to catch idle timeout events.
     *
     * @param msec The idle timeout to be registered in milliseconds.
     */
    void registerIdleTimeout(int msec);

    void unregisterIdleTimeout();

public Q_SLOTS:
    /**
     * This slot is triggered whenever an idle timeout registered with registerIdleTimeout is reached.
     *
     * @param msec The idle timeout reached in milliseconds
     */
    virtual void onIdleTimeout(int msec) = 0;
    /**
     * This slot is triggered whenever the PC wakes up from an Idle state. It is @b always called after a registered
     * idle timeout has been reached.
     */
    virtual void onWakeupFromIdle() = 0;

Q_SIGNALS:
    void actionTriggered(bool result, const QString &error = QString());

private:
    class Private;
    Private * const d;

    friend class IDleManager;
};

#endif // POWERDEVIL_POWERDEVILACTION_H
