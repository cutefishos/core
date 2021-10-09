/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Reion Wong <reion@cutefishos.com>
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
#include "notificationwindow.h"

class NotificationServer;
class NotificationsModel;
class Application : public QApplication
{
    Q_OBJECT

public:
    explicit Application(int& argc, char** argv);

    void showWindow();

    int run();
    bool parseCommandLineArgs();

private:
    NotificationServer *m_notificationServer;
    NotificationsModel *m_model;
    NotificationWindow *m_window;
    bool m_instance;
};

#endif // APPLICATION_H
