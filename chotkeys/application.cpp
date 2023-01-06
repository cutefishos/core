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

#include "application.h"
#include "hotkeys.h"

#include <QProcess>
#include <QDebug>

Application::Application(QObject *parent)
    : QObject(parent)
    , m_hotKeys(new Hotkeys)
{
    setupShortcuts();

    connect(m_hotKeys, &Hotkeys::pressed, this, &Application::onPressed);
    connect(m_hotKeys, &Hotkeys::released, this, &Application::onReleased);
}

void Application::setupShortcuts()
{
    m_hotKeys->registerKey(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_Delete));
    m_hotKeys->registerKey(QKeySequence(Qt::CTRL + Qt::ALT + Qt::Key_A));
    m_hotKeys->registerKey(QKeySequence(Qt::META + Qt::Key_L));
    //m_hotKeys->registerKey(QKeySequence(Qt::META + Qt::Key_6));
    m_hotKeys->registerKey(647);
}

void Application::onPressed(QKeySequence keySeq)
{

    if (keySeq.toString() == "Ctrl+Alt+Del") {
        QProcess::startDetached("cutefish-shutdown", QStringList());
    }

    if (keySeq.toString() == "Meta+L") {
        QProcess::startDetached("cutefish-screenlocker", QStringList());
    }

    if (keySeq.toString() == "Ctrl+Alt+A") {
        QProcess::startDetached("cutefish-screenshot", QStringList());
    }

    if (keySeq.toString() == "Ʇ") {
        QProcess::startDetached("cutefish-launcher", QStringList());
    }
}