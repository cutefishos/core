#ifndef DEFAULTAPPLICATIONS_H
#define DEFAULTAPPLICATIONS_H

#include <QObject>

class DefaultApplications : public QObject
{
    Q_OBJECT

public:
    explicit DefaultApplications(QObject *parent = nullptr);
};

#endif // DEFAULTAPPLICATIONS_H
