#include "view.h"

#include <QApplication>
#include <QQmlContext>

View::View(QQuickView *parent)
    : QQuickView(parent)
{
    rootContext()->setContextProperty("View", this);

    setFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    // setResizeMode(QQuickView::SizeViewToRootObject);
    setSource(QUrl("qrc:/qml/main.qml"));
    setScreen(qApp->primaryScreen());
    setColor(Qt::transparent);
}
