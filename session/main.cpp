#include "application.h"
#include <QQuickWindow>

int main(int argc, char *argv[])
{
    putenv((char *)"SESSION_MANAGER=");

    // force xcb QPA plugin as session manager server is very X11 specific.
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("xcb"));

    QQuickWindow::setDefaultAlphaBuffer(true);
    QCoreApplication::setAttribute(Qt::AA_DisableHighDpiScaling);

    Application a(argc, argv);
    a.setQuitOnLastWindowClosed(false);

    return a.exec();
}
