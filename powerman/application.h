/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reionwong@gmail.com>
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

#ifndef APPLICATION_H
#define APPLICATION_H

#include <QObject>
#include <QSettings>
#include <QAction>
#include <QSet>

#include "lidwatcher.h"
#include "idlemanager.h"
#include "dimdisplayaction.h"

#include "cpu/cpumanagement.h"

class Application : public QObject
{
    Q_OBJECT

public:
    explicit Application(QObject *parent = nullptr);

    void setDimDisplayTimeout(int timeout);
    void setSleepWhenClosedScreen(bool enabled);
    void setLockWhenClosedScreen(bool lock);

private:
    LidWatcher *m_lidWatcher;
    CPUManagement *m_cpuManagement;
    QSettings m_settings;

    int m_closeScreenTimeout;
    bool m_sleepWhenClosedScreen;
    bool m_lockWhenClosedScreen;

    DimDisplayAction *m_dimDisplayAction;
};

#endif // APPLICATION_H
