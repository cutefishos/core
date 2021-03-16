#ifndef ACTIONS_H
#define ACTIONS_H

#include <QObject>

class Actions : public QObject
{
    Q_OBJECT

public:
    explicit Actions(QObject *parent = nullptr);

    Q_INVOKABLE void shutdown();
    Q_INVOKABLE void logout();
    Q_INVOKABLE void reboot();
    Q_INVOKABLE void suspend();
};

#endif // ACTIONS_H
