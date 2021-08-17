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

#ifndef HOTKEYS_H
#define HOTKEYS_H

#include <QObject>
#include <QKeySequence>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QTimer>
#include <QSet>

#include <xcb/xcb.h>

class Hotkeys : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit Hotkeys(QObject *parent = nullptr);
    ~Hotkeys();

    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result) override;

    void registerKey(QKeySequence keySequence);
    void registerKey(quint32 keycode);

    void registerKey(quint32 key, quint32 mods);
    void unregisterKey(quint32 key, quint32 mods);


signals:
    void pressed(QKeySequence keySeq);
    void released(QKeySequence keySeq);

private:
    quint32 nativeKeycode(Qt::Key k);
    quint32 nativeModifiers(Qt::KeyboardModifiers m);
    Qt::Key getKey(const QKeySequence& keyseq);
    Qt::KeyboardModifiers getMods(const QKeySequence& keyseq);

private:
    QHash<quint32, QKeySequence> m_shortcuts;
};

#endif // HOTKEYS_H
