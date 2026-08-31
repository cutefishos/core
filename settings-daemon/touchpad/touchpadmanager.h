#ifndef TOUCHPADMANAGER_H
#define TOUCHPADMANAGER_H

#include <QObject>

class KWinInputBackend;

class TouchpadManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY availableChanged)
    Q_PROPERTY(bool enabled READ enabled WRITE setEnabled NOTIFY enabledChanged)
    Q_PROPERTY(bool tapToClick READ tapToClick WRITE setTapToClick NOTIFY tapToClickChanged)
    Q_PROPERTY(bool naturalScroll READ naturalScroll WRITE setNaturalScroll NOTIFY naturalScrollChanged)
    Q_PROPERTY(qreal pointerAcceleration READ pointerAcceleration WRITE setPointerAcceleration NOTIFY pointerAccelerationChanged)

public:
    explicit TouchpadManager(QObject *parent = nullptr);

    bool available() const;

    bool enabled() const;
    void setEnabled(bool enabled);

    bool tapToClick() const;
    void setTapToClick(bool value);

    bool naturalScroll() const;
    void setNaturalScroll(bool naturalScroll);

    qreal pointerAcceleration() const;
    void setPointerAcceleration(qreal value);

signals:
    void availableChanged();
    void enabledChanged();
    void tapToClickChanged();
    void naturalScrollChanged();
    void pointerAccelerationChanged();

private:
    KWinInputBackend *m_backend;
};

#endif // TOUCHPADMANAGER_H
