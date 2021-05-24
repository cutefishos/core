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

#ifndef PROCESSMANAGER_H
#define PROCESSMANAGER_H

#include <QAbstractNativeEventFilter>
#include <QObject>
#include <QProcess>
#include <QEventLoop>
#include <QMap>

class ProcessManager : public QObject, public QAbstractNativeEventFilter
{
    Q_OBJECT

public:
    explicit ProcessManager(QObject *parent = nullptr);
    ~ProcessManager();

    void start();
    void logout();

    void startWindowManager();
    void loadSystemProcess();
    void loadAutoStartProcess();

    bool nativeEventFilter(const QByteArray & eventType, void * message, long * result) override;

private:
    QMap<QString, QProcess *> m_systemProcess;
    QMap<QString, QProcess *> m_autoStartProcess;

    bool m_wmStarted;
    QEventLoop *m_waitLoop;
};

#endif // PROCESSMANAGER_H
