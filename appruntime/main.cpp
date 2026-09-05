#include "appruntime.h"
#include "appruntimeadaptor.h"

#include <QCoreApplication>
#include <QDBusConnection>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("cutefish-appruntime"));

    AppRuntime runtime;
    new AppRuntimeAdaptor(&runtime);

    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.registerService(QStringLiteral("com.cutefish.AppRuntime"))) {
        qWarning() << "Another application runtime is already running";
        return 1;
    }

    bus.registerObject(QStringLiteral("/AppRuntime"), &runtime);

    return app.exec();
}
