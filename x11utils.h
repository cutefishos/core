#pragma once

#include <QGuiApplication>
#include <QtGui/qguiapplication_platform.h>

#include <xcb/xcb.h>

namespace Cutefish::X11
{
inline xcb_window_t rootWindow()
{
    const auto native = qGuiApp ? qGuiApp->nativeInterface<QNativeInterface::QX11Application>() : nullptr;
    const auto c = native ? native->connection() : nullptr;
    if (!c)
        return XCB_WINDOW_NONE;

    const auto *setup = xcb_get_setup(c);
    const auto iterator = xcb_setup_roots_iterator(setup);
    return iterator.data ? iterator.data->root : XCB_WINDOW_NONE;
}

inline int screenNumber()
{
    return 0;
}
}
