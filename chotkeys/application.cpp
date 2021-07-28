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
#include <KGlobalAccel>
#include <QAction>
#include <QProcess>

Application::Application(QObject *parent)
    : QObject(parent)
{
    setupShortcuts();
}

void Application::setupShortcuts()
{
    QAction *a = addAction("Log Out");
    KGlobalAccel::self()->setGlobalShortcut(a, QList<QKeySequence>() << Qt::ALT + Qt::CTRL + Qt::Key_Delete);
    connect(a, &QAction::triggered, this, [=] { QProcess::startDetached("cutefish-shutdown", QStringList()); });

    a = addAction("Lock Screen");
    a->setProperty("isConfigurationAction", true);
    KGlobalAccel::self()->setDefaultShortcut(a, QList<QKeySequence>() << Qt::META + Qt::Key_L);
    KGlobalAccel::self()->setGlobalShortcut(a, QList<QKeySequence>() << Qt::META + Qt::Key_L);
    connect(a, &QAction::triggered, this, [=] { QProcess::startDetached("cutefish-screenlocker", QStringList()); });
}

QAction *Application::addAction(const QString &name)
{
    QAction *a = new QAction(this);
    a->setProperty("componentDisplayName", QStringLiteral("kwin"));
    a->setObjectName(name);
    a->setText(name);
    return a;
}
