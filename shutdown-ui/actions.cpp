#include "actions.h"
#include <QCommandLineParser>
#include <QDBusInterface>
#include <QApplication>

const static QString s_dbusName = "org.cutefish.Session";
const static QString s_pathName = "/Session";
const static QString s_interfaceName = "org.cutefish.Session";

Actions::Actions(QObject *parent)
    : QObject(parent)
{

}

void Actions::shutdown()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("powerOff");
    }
}

void Actions::logout()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("logout");
    }
}

void Actions::reboot()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("reboot");
    }
}

void Actions::suspend()
{
    QDBusInterface iface(s_dbusName, s_pathName, s_interfaceName, QDBusConnection::sessionBus());
    if (iface.isValid()) {
        iface.call("suspend");
    }
    QCoreApplication::exit(0);
}
