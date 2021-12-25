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

#ifndef POLKITAGENTLISTENER_H
#define POLKITAGENTLISTENER_H

#include <QObject>
#include <QString>
#include <QPointer>

#include <PolkitQt1/Identity>
#include <PolkitQt1/Details>
#include <PolkitQt1/Agent/Listener>
#include <PolkitQt1/Agent/Session>

class Dialog;
class PolKitAgentListener : public PolkitQt1::Agent::Listener
{
    Q_OBJECT

public:
    explicit PolKitAgentListener(QObject *parent = nullptr);

public slots:
    void initiateAuthentication(const QString &actionId,
                                const QString &message,
                                const QString &iconName,
                                const PolkitQt1::Details &details,
                                const QString &cookie,
                                const PolkitQt1::Identity::List &identities,
                                PolkitQt1::Agent::AsyncResult *result);

    bool initiateAuthenticationFinish() { return true; }
    void cancelAuthentication() {}

    void request(const QString &request, bool echo);
    void completed(bool gainedAuthorization);

private:
    void tryAgain();
    void finishObtainPrivilege();

private slots:
    void onDialogAccepted();
    void onDialogCanceled();

private:
    QPointer<Dialog> m_dialog;
    QPointer<PolkitQt1::Agent::Session> m_session;
    PolkitQt1::Identity::List m_identities;
    PolkitQt1::Agent::AsyncResult *m_result;
    PolkitQt1::Identity m_selectedUser;
    QString m_cookie;

    bool m_inProgress;
    bool m_gainedAuthorization;
    bool m_wasCancelled;

    int m_numTries;
};

#endif // POLKITAGENTLISTENER_H
