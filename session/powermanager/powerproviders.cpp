/* BEGIN_COMMON_COPYRIGHT_HEADER
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *   Petr Vanek <petr@scribus.info>
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


#include "powerproviders.h"
#include <QDBusInterface>
#include <QProcess>
#include <QDebug>
#include <signal.h> // for kill()

#define UPOWER_SERVICE          "org.freedesktop.UPower"
#define UPOWER_PATH             "/org/freedesktop/UPower"
#define UPOWER_INTERFACE        UPOWER_SERVICE

#define CONSOLEKIT_SERVICE      "org.freedesktop.ConsoleKit"
#define CONSOLEKIT_PATH         "/org/freedesktop/ConsoleKit/Manager"
#define CONSOLEKIT_INTERFACE    "org.freedesktop.ConsoleKit.Manager"

#define SYSTEMD_SERVICE         "org.freedesktop.login1"
#define SYSTEMD_PATH            "/org/freedesktop/login1"
#define SYSTEMD_INTERFACE       "org.freedesktop.login1.Manager"

#define PROPERTIES_INTERFACE    "org.freedesktop.DBus.Properties"

/************************************************
 Helper func
 ************************************************/
void printDBusMsg(const QDBusMessage &msg)
{
    qWarning() << "** Dbus error **************************";
    qWarning() << "Error name " << msg.errorName();
    qWarning() << "Error msg  " << msg.errorMessage();
    qWarning() << "****************************************";
}

/************************************************
 Helper func
 ************************************************/
static bool dbusCall(const QString &service,
              const QString &path,
              const QString &interface,
              const QDBusConnection &connection,
              const QString & method,
              PowerProvider::DbusErrorCheck errorCheck = PowerProvider::CheckDBUS
              )
{
    QDBusInterface dbus(service, path, interface, connection);
    
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        if (errorCheck == PowerProvider::CheckDBUS)
        {
            // Notification::notify(
            //                         QObject::tr("Power Manager Error"),
            //                         QObject::tr("QDBusInterface is invalid") + QStringLiteral("\n\n") + service + QStringLiteral(' ') + path + QStringLiteral(' ') + interface + QStringLiteral(' ') + method,
            //                         QStringLiteral("logo.png"));
        }
        return false;
    }

    QDBusMessage msg = dbus.call(method);
    if (!msg.errorName().isEmpty()) {
        printDBusMsg(msg);
        if (errorCheck == PowerProvider::CheckDBUS)
        {
            // Notification::notify(
            //                         QObject::tr("Power Manager Error (D-BUS call)"),
            //                         msg.errorName() + QStringLiteral("\n\n") + msg.errorMessage(),
            //                         QStringLiteral("logo.png"));
        }
    }

    // If the method no returns value, we believe that it was successful.
    return msg.arguments().isEmpty() ||
           msg.arguments().constFirst().isNull() ||
           msg.arguments().constFirst().toBool();
}

/************************************************
 Helper func

 Just like dbusCall(), except that systemd
 returns a string instead of a bool, and it takes
 an "interactivity boolean" as an argument.
 ************************************************/
static bool dbusCallSystemd(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString &method,
                     bool needBoolArg,
                     PowerProvider::DbusErrorCheck errorCheck = PowerProvider::CheckDBUS
                     )
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid()) {
        qWarning() << "dbusCall: QDBusInterface is invalid" << service << path << interface << method;
        if (errorCheck == PowerProvider::CheckDBUS) {
            // Notification::notify(
            //                         QObject::tr("Power Manager Error"),
            //                         QObject::tr("QDBusInterface is invalid") + QStringLiteral("\n\n") + service + QStringLiteral(' ') + path + QStringLiteral(' ')+ interface + QStringLiteral(' ') + method,
            //                         QStringLiteral("logo.png"));
        }

        return false;
    }

    QDBusMessage msg = dbus.call(method, needBoolArg ? QVariant(true) : QVariant());

    if (!msg.errorName().isEmpty()) {
        printDBusMsg(msg);
        if (errorCheck == PowerProvider::CheckDBUS) {
            // Notification::notify(
            //                         QObject::tr("Power Manager Error (D-BUS call)"),
            //                         msg.errorName() + QStringLiteral("\n\n") + msg.errorMessage(),
            //                         QStringLiteral("logo.png"));
        }
    }

    // If the method no returns value, we believe that it was successful.
    if (msg.arguments().isEmpty() || msg.arguments().constFirst().isNull())
        return true;

    QString response = msg.arguments().constFirst().toString();
    qDebug() << "systemd:" << method << "=" << response;
    return response == QStringLiteral("yes") || response == QStringLiteral("challenge");
}

/************************************************
 Helper func
 ************************************************/
bool dbusGetProperty(const QString &service,
                     const QString &path,
                     const QString &interface,
                     const QDBusConnection &connection,
                     const QString & property
                    )
{
    QDBusInterface dbus(service, path, interface, connection);
    if (!dbus.isValid())
    {
        qWarning() << "dbusGetProperty: QDBusInterface is invalid" << service << path << interface << property;
//        Notification::notify(QObject::tr("Power Manager"),
//                                  "logo.png",
//                                  QObject::tr("Power Manager Error"),
//                                  QObject::tr("QDBusInterface is invalid")+ "\n\n" + service +" " + path +" " + interface +" " + property);

        return false;
    }

    QDBusMessage msg = dbus.call(QStringLiteral("Get"), dbus.interface(), property);

    if (!msg.errorName().isEmpty())
    {
        printDBusMsg(msg);
//        Notification::notify(QObject::tr("Power Manager"),
//                                  "logo.png",
//                                  QObject::tr("Power Manager Error (Get Property)"),
//                                  msg.errorName() + "\n\n" + msg.errorMessage());
    }

    return !msg.arguments().isEmpty() &&
            msg.arguments().constFirst().value<QDBusVariant>().variant().toBool();
}

/************************************************
 PowerProvider
 ************************************************/
PowerProvider::PowerProvider(QObject *parent):
    QObject(parent)
{
}

PowerProvider::~PowerProvider()
{
}

/************************************************
 UPowerProvider
 ************************************************/
UPowerProvider::UPowerProvider(QObject *parent):
    PowerProvider(parent)
{
}

UPowerProvider::~UPowerProvider()
{
}

bool UPowerProvider::canAction(Power::Action action) const
{
    QString command;
    QString property;
    switch (action) {
    case Power::PowerHibernate:
        property = QStringLiteral("CanHibernate");
        command  = QStringLiteral("HibernateAllowed");
        break;
    case Power::PowerSuspend:
        property = QStringLiteral("CanSuspend");
        command  = QStringLiteral("SuspendAllowed");
        break;
    default:
        return false;
    }

    return dbusGetProperty(  // Whether the system is able to hibernate.
                QStringLiteral(UPOWER_SERVICE),
                QStringLiteral(UPOWER_PATH),
                QStringLiteral(PROPERTIES_INTERFACE),
                QDBusConnection::systemBus(),
                property
            )
            &&
            dbusCall( // Check if the caller has (or can get) the PolicyKit privilege to call command.
                QStringLiteral(UPOWER_SERVICE),
                QStringLiteral(UPOWER_PATH),
                QStringLiteral(UPOWER_INTERFACE),
                QDBusConnection::systemBus(),
                command,
                // canAction should be always silent because it can freeze
                // g_main_context_iteration Qt event loop in QMessageBox
                // on panel startup if there is no DBUS running.
                PowerProvider::DontCheckDBUS
            );
}

bool UPowerProvider::doAction(Power::Action action)
{
    QString command;

    switch (action) {
    case Power::PowerHibernate:
        command = QStringLiteral("Hibernate");
        break;
    case Power::PowerSuspend:
        command = QStringLiteral("Suspend");
        break;
    default:
        return false;
    }


    return dbusCall(QStringLiteral(UPOWER_SERVICE),
             QStringLiteral(UPOWER_PATH),
             QStringLiteral(UPOWER_INTERFACE),
             QDBusConnection::systemBus(),
             command );
}

/************************************************
 ConsoleKitProvider
 ************************************************/
ConsoleKitProvider::ConsoleKitProvider(QObject *parent):
    PowerProvider(parent)
{
}

ConsoleKitProvider::~ConsoleKitProvider()
{
}

bool ConsoleKitProvider::canAction(Power::Action action) const
{
    QString command;
    switch (action) {
    case Power::PowerReboot:
        command = QStringLiteral("CanReboot");
        break;

    case Power::PowerShutdown:
        command = QStringLiteral("CanPowerOff");
        break;

    case Power::PowerHibernate:
        command  = QStringLiteral("CanHibernate");
        break;

    case Power::PowerSuspend:
        command  = QStringLiteral("CanSuspend");
        break;

    default:
        return false;
    }

    return dbusCallSystemd(QStringLiteral(CONSOLEKIT_SERVICE),
                    QStringLiteral(CONSOLEKIT_PATH),
                    QStringLiteral(CONSOLEKIT_INTERFACE),
                    QDBusConnection::systemBus(),
                    command,
                    false,
                    // canAction should be always silent because it can freeze
                    // g_main_context_iteration Qt event loop in QMessageBox
                    // on panel startup if there is no DBUS running.
                    PowerProvider::DontCheckDBUS
                   );
}

bool ConsoleKitProvider::doAction(Power::Action action)
{
    QString command;
    switch (action) {
    case Power::PowerReboot:
        command = QStringLiteral("Reboot");
        break;
    case Power::PowerShutdown:
        command = QStringLiteral("PowerOff");
        break;
    case Power::PowerHibernate:
        command = QStringLiteral("Hibernate");
        break;
    case Power::PowerSuspend:
        command = QStringLiteral("Suspend");
        break;
    default:
        return false;
    }

    return dbusCallSystemd(QStringLiteral(CONSOLEKIT_SERVICE),
                QStringLiteral(CONSOLEKIT_PATH),
                QStringLiteral(CONSOLEKIT_INTERFACE),
                QDBusConnection::systemBus(),
                command,
                true);
}

/************************************************
  SystemdProvider

  http://www.freedesktop.org/wiki/Software/systemd/logind
 ************************************************/

SystemdProvider::SystemdProvider(QObject *parent):
    PowerProvider(parent)
{
}

SystemdProvider::~SystemdProvider()
{
}

bool SystemdProvider::canAction(Power::Action action) const
{
    QString command;

    switch (action) {
    case Power::PowerReboot:
        command = QStringLiteral("CanReboot");
        break;
    case Power::PowerShutdown:
        command = QStringLiteral("CanPowerOff");
        break;
    case Power::PowerSuspend:
        command = QStringLiteral("CanSuspend");
        break;
    case Power::PowerHibernate:
        command = QStringLiteral("CanHibernate");
        break;
    default:
        return false;
    }

    return dbusCallSystemd(QStringLiteral(SYSTEMD_SERVICE),
                    QStringLiteral(SYSTEMD_PATH),
                    QStringLiteral(SYSTEMD_INTERFACE),
                    QDBusConnection::systemBus(),
                    command,
                    false,
                    // canAction should be always silent because it can freeze
                    // g_main_context_iteration Qt event loop in QMessageBox
                    // on panel startup if there is no DBUS running.
                    PowerProvider::DontCheckDBUS
                   );
}

bool SystemdProvider::doAction(Power::Action action)
{
    QString command;

    switch (action) {
    case Power::PowerReboot:
        command = QStringLiteral("Reboot");
        break;
    case Power::PowerShutdown:
        command = QStringLiteral("PowerOff");
        break;
    case Power::PowerSuspend:
        command = QStringLiteral("Suspend");
        break;
    case Power::PowerHibernate:
        command = QStringLiteral("Hibernate");
        break;
    default:
        return false;
    }

    return dbusCallSystemd(QStringLiteral(SYSTEMD_SERVICE),
             QStringLiteral(SYSTEMD_PATH),
             QStringLiteral(SYSTEMD_INTERFACE),
             QDBusConnection::systemBus(),
             command,
             true
            );
}

/************************************************
  HalProvider
 ************************************************/
HalProvider::HalProvider(QObject *parent):
    PowerProvider(parent)
{
}

HalProvider::~HalProvider()
{
}

bool HalProvider::canAction(Power::Action action) const
{
    Q_UNUSED(action)
    return false;
}

bool HalProvider::doAction(Power::Action action)
{
    Q_UNUSED(action)
    return false;
}
