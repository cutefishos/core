#ifndef VIEW_H
#define VIEW_H

#include <QQuickView>

class View : public QQuickView
{
    Q_OBJECT

public:
    explicit View(QQuickView *parent = nullptr);

};

#endif // VIEW_H
