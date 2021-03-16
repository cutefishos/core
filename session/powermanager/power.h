/* BEGIN_COMMON_COPYRIGHT_HEADER
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#ifndef POWER_H
#define POWER_H

#include <QObject>
#include <QList>

class PowerProvider;

/*! Power class provides an interface to control system-wide power and session management.
    It allows logout from the user session, hibernate, reboot, shutdown and suspend computer.
    This is a wrapper class. All the real work is done in the PowerWorker classes.
*/
class Power : public QObject
{
    Q_OBJECT

public:
    /// Power can perform next actions:
    enum Action{
        PowerLogout,    /// Close the current user session.
        PowerHibernate, /// Hibernate the comupter
        PowerReboot,    /// Reboot the computer
        PowerShutdown,  /// Shutdown the computer
        PowerSuspend,   /// Suspend the computer
        PowerMonitorOff, /// Turn off the monitor(s)
        PowerShowLeaveDialog /// Show the lxqt-leave dialog
    };

    /*!
     * Constructs the Power object.
     * \param useLxqtSessionProvider indicates if the DBus methods
     * provided by lxqt-session should be considered. This is useful to
     * avoid recursion if the lxqt-session wants to provide some of the
     * methods by itself with internal use of this object.
     */
    explicit Power(bool useSessionProvider, QObject *parent = nullptr);
    /// Constructs a Power with using the lxqt-session provider.
    explicit Power(QObject *parent = nullptr);

    /// Destroys the object.
    ~Power() override;

    /// Returns true if the Power can perform action.
    bool canAction(Action action) const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerLogout).
    bool canLogout() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerHibernate).
    bool canHibernate() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerReboot).
    bool canReboot() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerShutdown).
    bool canShutdown() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerSuspend).
    bool canSuspend() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerMonitorOff).
    bool canMonitorOff() const;

    //! This function is provided for convenience. It's equivalent to calling canAction(PowerShowLeaveDialog).
    bool canShowLeaveDialog() const;

public Q_SLOTS:
    /// Performs the requested action.
    bool doAction(Action action);

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerLogout).
    bool logout();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerHibernate).
    bool hibernate();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerReboot).
    bool reboot();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerShutdown).
    bool shutdown();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerSuspend).
    bool suspend();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerMonitorOff).
    bool monitorOff();

    //! This function is provided for convenience. It's equivalent to calling doAction(PowerShowLeaveDialog).
    bool showLeaveDialog();

private:
    QList<PowerProvider*> m_providers;
};

#endif
