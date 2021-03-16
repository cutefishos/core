#include <QGuiApplication>
#include <PolkitQt1/Subject>
#include <QFile>
#include <QLocale>
#include <QTranslator>

#include "polkitagentlistener.h"

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QGuiApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    // Translations
    QLocale locale;
    QString qmFilePath = QString("%1/%2.qm").arg("/usr/share/cutefish-polkit-agent/translations/").arg(locale.name());
    if (QFile::exists(qmFilePath)) {
        QTranslator *translator = new QTranslator(QGuiApplication::instance());
        if (translator->load(qmFilePath)) {
            QGuiApplication::installTranslator(translator);
        } else {
            translator->deleteLater();
        }
    }

    PolKitAgentListener listener;
    PolkitQt1::UnixSessionSubject session(getpid());

    if (!listener.registerListener(session, QStringLiteral("/org/cutefishos/PolicyKit1/AuthenticationAgent")))
        return -1;

    return app.exec();
}
