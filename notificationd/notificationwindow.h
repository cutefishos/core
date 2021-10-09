#ifndef NOTIFICATIONWINDOW_H
#define NOTIFICATIONWINDOW_H

#include <QQuickView>

class NotificationWindow : public QQuickView
{
    Q_OBJECT

public:
    explicit NotificationWindow(QQuickView *parent = nullptr);

    void open();

    bool eventFilter(QObject *object, QEvent *event) override;
};

#endif // NOTIFICATIONWINDOW_H
