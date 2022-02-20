/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     revenmartin <revenmartin@gmail.com>
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

#include <QApplication>
#include "theme/thememanager.h"
#include "brightness/brightnessmanager.h"
#include "battery/upowermanager.h"
#include "language/language.h"
#include "mouse/mousemanager.h"
#include "touchpad/touchpadmanager.h"
#include "defaultapplications.h"

#include <QTimer>

class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int &argc, char **argv);

    void invokeDesktopProcess();
    void initTrash();

//    void initKWin();

private:
    ThemeManager *m_themeManager;
    BrightnessManager *m_brightnessManager;
    UPowerManager *m_upowerManager;
    Language *m_language;
    Mouse *m_mouse;
    TouchpadManager *m_touchpad;
    DefaultApplications *m_defaultApps;

//    QTimer *m_kwinTimer;
};

#endif // APPLICATION_H
