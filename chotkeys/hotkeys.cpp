/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <aj@cutefishos.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "hotkeys.h"

#include <QApplication>
#include <QKeySequence>
#include <QX11Info>
#include <QTimer>
#include <QDebug>

// #include <KKeyServer>
// #include <NETWM>

// XCB & X11
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <xcb/xcb_keysyms.h>

#include <X11/XKBlib.h>

Hotkeys::Hotkeys(QObject *parent)
    : QObject(parent)
{
    qApp->installNativeEventFilter(this);
}

Hotkeys::~Hotkeys()
{
    qApp->removeNativeEventFilter(this);
}

bool Hotkeys::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    Q_UNUSED(result);

    if (eventType != "xcb_generic_event_t") {
        return false;
    }

    xcb_generic_event_t *e = static_cast<xcb_generic_event_t *>(message);

    if (e->response_type == XCB_KEY_PRESS) {
        xcb_key_press_event_t *keyEvent = static_cast<xcb_key_press_event_t *>(message);
        quint32 keycode = keyEvent->detail;
        quint32 mods = keyEvent->state; // & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask);
        quint32 id = keycode | mods;

//        int keyQt;
//        KKeyServer::xcbKeyPressEventToQt(keyEvent, &keyQt);

//        bool found = false;
//        for (QKeySequence &seq : m_shortcuts.values()) {
//            if (seq == QKeySequence(keyQt)) {
//                found = true;
//                // emit pressed(seq);
//                break;
//            }
//        }

//        if (!found && m_shortcuts.contains(id)) {
//            // emit pressed(m_shortcuts[id]);
//        }

//        // Keyboard needs to be ungrabed after XGrabKey() activates the grab,
//        // otherwise it becomes frozen.
//        xcb_connection_t *c = QX11Info::connection();
//        xcb_void_cookie_t cookie = xcb_ungrab_keyboard_checked(c, XCB_TIME_CURRENT_TIME);
//        xcb_flush(c);

//        // xcb_flush() only makes sure that the ungrab keyboard request has been
//        // sent, but is not enough to make sure that request has been fulfilled. Use
//        // xcb_request_check() to make sure that the request has been processed.
//        xcb_request_check(c, cookie);

//        int keyQt;
//        if (!KKeyServer::xcbKeyPressEventToQt(keyEvent, &keyQt)) {
//            qDebug() << "KKeyServer::xcbKeyPressEventToQt failed";
//            return false;
//        }

//        // All that work for this hey... argh...
//        if (NET::timestampCompare(keyEvent->time, QX11Info::appTime()) > 0) {
//            QX11Info::setAppTime(keyEvent->time);
//        }

//        bool found = false;
//        for (QKeySequence &seq : m_shortcuts.values()) {
//            if (seq == QKeySequence(keyQt)) {
//                found = true;
//                emit pressed(seq);
//                break;
//            }
//        }

        if (m_shortcuts.contains(id)) {
            emit pressed(m_shortcuts[id]);
        }

        return true;

    } else if (e->response_type == XCB_KEY_RELEASE) {
        xcb_key_release_event_t *keyEvent = static_cast<xcb_key_release_event_t *>(message);
        quint32 keycode = keyEvent->detail;
        quint32 mods = keyEvent->state; // & (ShiftMask | ControlMask | Mod1Mask | Mod3Mask);
        quint32 id = keycode | mods;

        // META
        if (id == 197) {
            id = 133;
        }

        if (m_shortcuts.contains(id)) {
            emit released(m_shortcuts[id]);
        }

        return true;
    }

    return false;
}

void Hotkeys::registerKey(QKeySequence keySequence)
{
    if (keySequence.isEmpty())
        return;

    quint32 keycode = nativeKeycode(getKey(keySequence));
    quint32 mods = nativeModifiers(getMods(keySequence));
    quint32 keyId = keycode | mods;

    // META
    if (keycode == 204 && mods == 0) {
        keycode = 133;
        keyId = keycode | mods;
    }

    if (!m_shortcuts.contains(keyId)) {
        registerKey(keycode, mods);
        m_shortcuts.insert(keyId, keySequence);
    }
}

void Hotkeys::registerKey(quint32 keycode)
{
    if (!m_shortcuts.contains(keycode)) {
        registerKey(keycode, 0);
        m_shortcuts.insert(keycode, QKeySequence(keycode | 0));
    }
}

void Hotkeys::registerKey(quint32 key, quint32 mods)
{
    xcb_grab_key(QX11Info::connection(),
                 1,
                 QX11Info::appRootWindow(),
                 mods,
                 key,
                 XCB_GRAB_MODE_ASYNC,
                 XCB_GRAB_MODE_ASYNC);

    xcb_grab_key(QX11Info::connection(),
                 1,
                 QX11Info::appRootWindow(),
                 mods | XCB_MOD_MASK_2,
                 key,
                 XCB_GRAB_MODE_ASYNC,
                 XCB_GRAB_MODE_ASYNC);
}

void Hotkeys::unregisterKey(quint32 key, quint32 mods)
{
    xcb_ungrab_key(QX11Info::connection(), key, QX11Info::appRootWindow(), mods);
}

quint32 Hotkeys::nativeKeycode(Qt::Key k)
{
    /* keysymdef.h */
    quint32 key = 0;
    if (k >= Qt::Key_F1 && k <= Qt::Key_F35) {
        key = XK_F1 + (k - Qt::Key_F1);
    } else if (k >= Qt::Key_Space && k <= Qt::Key_QuoteLeft) {
        key = k;
    } else if (k >= Qt::Key_BraceLeft && k <= Qt::Key_AsciiTilde) {
        key = k;
    } else if (k >= Qt::Key_nobreakspace && k <= Qt::Key_ydiaeresis) {
        key = k;
    } else {
        switch (k) {
        case Qt::Key_Escape:
            key = XK_Escape;
            break;
        case Qt::Key_Tab:
        case Qt::Key_Backtab:
            key = XK_Tab;
            break;
        case Qt::Key_Backspace:
            key = XK_BackSpace;
            break;
        case Qt::Key_Return:
        case Qt::Key_Enter:
            key = XK_Return;
            break;
        case Qt::Key_Insert:
            key = XK_Insert;
            break;
        case Qt::Key_Delete:
            key = XK_Delete;
            break;
        case Qt::Key_Pause:
            key = XK_Pause;
            break;
        case Qt::Key_Print:
            key = XK_Print;
            break;
        case Qt::Key_SysReq:
            key = XK_Sys_Req;
            break;
        case Qt::Key_Clear:
            key = XK_Clear;
            break;
        case Qt::Key_Home:
            key = XK_Home;
            break;
        case Qt::Key_End:
            key = XK_End;
            break;
        case Qt::Key_Left:
            key = XK_Left;
            break;
        case Qt::Key_Up:
            key = XK_Up;
            break;
        case Qt::Key_Right:
            key = XK_Right;
            break;
        case Qt::Key_Down:
            key = XK_Down;
            break;
        case Qt::Key_PageUp:
            key = XK_Page_Up;
            break;
        case Qt::Key_PageDown:
            key = XK_Page_Down;
            break;
        default:
            key = 0;
        }
    }
    return XKeysymToKeycode(QX11Info::display(), key);
}

quint32 Hotkeys::nativeModifiers(Qt::KeyboardModifiers m)
{
    quint32 mods = Qt::NoModifier;

    if (m & Qt::ShiftModifier)
        mods |= ShiftMask;
    if (m & Qt::ControlModifier)
        mods |= ControlMask;
    if (m & Qt::AltModifier)
        mods |= Mod1Mask;
    if (m & Qt::MetaModifier)
        mods |= Mod4Mask;

    return mods;
}

Qt::Key Hotkeys::getKey(const QKeySequence &keyseq)
{
    if (keyseq.isEmpty()) {
        return Qt::Key(0);
    }

    return Qt::Key(keyseq[0] & ~Qt::KeyboardModifierMask);
}

Qt::KeyboardModifiers Hotkeys::getMods(const QKeySequence &keyseq)
{
    if (keyseq.isEmpty()) {
        return Qt::KeyboardModifiers();
    }

    return Qt::KeyboardModifiers(keyseq[0] & Qt::KeyboardModifierMask);
}
