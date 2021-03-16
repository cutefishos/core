#include "dialog.h"

#include <QGuiApplication>
#include <QQmlContext>
#include <QScreen>
#include <QDebug>

Dialog::Dialog(const QString &action, const QString &message,
               const QString &cookie, const QString &identity,
               PolkitQt1::Agent::AsyncResult *result)
    : m_action(action)
    , m_message(message)
    , m_cookie(cookie)
    , m_identity(identity)
    , m_password(QString())
    , m_result(result)
    , m_view(new QQuickView)
{
    qDebug() << "Creating ConfirmationDialog";

    m_view->setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    m_view->rootContext()->setContextProperty("confirmation", this);
    m_view->rootContext()->setContextProperty("rootWindow", m_view);
    m_view->setResizeMode(QQuickView::SizeViewToRootObject);
    m_view->setClearBeforeRendering(true);
    m_view->setDefaultAlphaBuffer(true);
    m_view->setColor(Qt::transparent);
    m_view->setSource(QUrl(QStringLiteral("qrc:/main.qml")));
    m_view->setVisible(false);
}

Dialog::~Dialog()
{
    qDebug() << "Deleting ConfirmationDialog";

    delete m_view;
}

void Dialog::setConfirmationResult(const QString &passwd)
{
    m_password = passwd;

    emit finished(this);
}

void Dialog::show()
{
    m_view->show();
    m_view->setScreen(qGuiApp->primaryScreen());
    m_view->setX((m_view->screen()->geometry().width() - m_view->geometry().width()) / 2);
    m_view->setY((m_view->screen()->geometry().height() - m_view->geometry().height()) / 2);
}
