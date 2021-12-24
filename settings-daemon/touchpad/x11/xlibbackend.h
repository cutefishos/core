/*
    SPDX-FileCopyrightText: 2013 Alexander Mezin <mezin.alexander@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef XLIBBACKEND_H
#define XLIBBACKEND_H

#include <QLatin1String>
#include <QMap>
#include <QScopedPointer>
#include <QSet>
#include <QSharedPointer>
#include <QStringList>
#include <QX11Info>

#include "libinputtouchpad.h"
#include "synapticstouchpad.h"
#include "xlibtouchpad.h"

#include <xcb/xcb.h>

#include "propertyinfo.h"
#include "xcbatom.h"

class XlibTouchpad;
class XlibNotifications;
class XRecordKeyboardMonitor;

class XlibBackend : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int touchpadCount READ touchpadCount CONSTANT)

public:
    enum TouchpadOffState {
        TouchpadEnabled,
        TouchpadTapAndScrollDisabled,
        TouchpadFullyDisabled,
    };

    static XlibBackend *initialize(QObject *parent = nullptr);
    ~XlibBackend();

    bool applyConfig(const QVariantHash &);
    bool applyConfig();
    bool getConfig(QVariantHash &);
    bool getConfig();
    bool getDefaultConfig();
    bool isChangedConfig() const;
    QStringList supportedParameters() const
    {
        return m_device ? m_device->supportedParameters() : QStringList();
    }
    QString errorString() const
    {
        return m_errorString;
    }
    int touchpadCount() const
    {
        return m_device ? 1 : 0;
    }

    void setTouchpadOff(TouchpadOffState);
    TouchpadOffState getTouchpadOff();

    bool isTouchpadAvailable();
    bool isTouchpadEnabled();
    void setTouchpadEnabled(bool);

    bool tapToClick();
    void setTapToClick(bool enabled);

    bool naturalScroll();
    void setNaturalScroll(bool value);

    qreal pointerAcceleration();
    void setPointerAcceleration(qreal value);

    void watchForEvents(bool keyboard);

    QStringList listMouses(const QStringList &blacklist);
    QVector<QObject *> getDevices() const;

signals:
    void touchpadStateChanged();
    void mousesChanged();
    void touchpadReset();
    void keyboardActivityStarted();
    void keyboardActivityFinished();

    void touchpadAdded(bool success);
    void touchpadRemoved(int index);

private Q_SLOTS:
    void propertyChanged(xcb_atom_t);
    void touchpadDetached();
    void devicePlugged(int);

protected:
    explicit XlibBackend(QObject *parent);

    struct XDisplayCleanup {
        static void cleanup(Display *);
    };

    QScopedPointer<Display, XDisplayCleanup> m_display;
    xcb_connection_t *m_connection;

    XcbAtom m_enabledAtom, m_mouseAtom, m_keyboardAtom, m_touchpadAtom;
    XcbAtom m_synapticsIdentifierAtom;
    XcbAtom m_libinputIdentifierAtom;

    XlibTouchpad *findTouchpad();
    QScopedPointer<XlibTouchpad> m_device;

    QString m_errorString;
    QScopedPointer<XlibNotifications> m_notifications;
    QScopedPointer<XRecordKeyboardMonitor> m_keyboard;
};

#endif // XLIBBACKEND_H
