#include "notificationwindow.h"
#include "notificationsmodel.h"
#include "historymodel.h"

#include <QQmlContext>

#include <KWindowSystem>
#include <KWindowEffects>

NotificationWindow::NotificationWindow(QQuickView *parent)
    : QQuickView(parent)
{
    installEventFilter(this);

    setFlags(Qt::Popup);
    setResizeMode(QQuickView::SizeRootObjectToView);
    setColor(Qt::transparent);

    rootContext()->setContextProperty("NotificationDialog", this);
    rootContext()->setContextProperty("notificationsModel", NotificationsModel::self());
    rootContext()->setContextProperty("historyModel", HistoryModel::self());

    // KWindowEffects::slideWindow(winId(), KWindowEffects::RightEdge);
    setSource(QUrl("qrc:/qml/NotificationWindow.qml"));
    setVisible(false);
}

void NotificationWindow::open()
{
    setVisible(true);
    setMouseGrabEnabled(true);
    setKeyboardGrabEnabled(true);
}

bool NotificationWindow::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress) {
        if (QWindow *w = qobject_cast<QWindow*>(object)) {
            if (!w->geometry().contains(static_cast<QMouseEvent*>(event)->globalPos())) {
                QQuickView::setVisible(false);
            }
        }
    } else if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Escape) {
            QQuickView::setVisible(false);
        }
    } else if (event->type() == QEvent::Show) {
        KWindowSystem::setState(winId(), NET::SkipTaskbar | NET::SkipPager | NET::SkipSwitcher);
        HistoryModel::self()->updateTime();
    }

    return QObject::eventFilter(object, event);
}
