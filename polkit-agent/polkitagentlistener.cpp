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

#include "polkitagentlistener.h"
#include "dialog.h"

#include <QDebug>

PolKitAgentListener::PolKitAgentListener(QObject *parent)
    : PolkitQt1::Agent::Listener(parent)
    , m_dialog(nullptr)
    , m_inProgress(false)
{

}

void PolKitAgentListener::initiateAuthentication(const QString &actionId,
                                                 const QString &message,
                                                 const QString &iconName,
                                                 const PolkitQt1::Details &details,
                                                 const QString &cookie,
                                                 const PolkitQt1::Identity::List &identities,
                                                 PolkitQt1::Agent::AsyncResult *result)
{
    Q_UNUSED(details);

    if (m_inProgress) {
        result->setCompleted();
        return;
    }

    m_identities = identities;
    m_cookie = cookie;
    m_result = result;
    m_session.clear();

    m_inProgress = true;

    m_dialog = new Dialog(actionId, message, cookie, identities.first().toString(), iconName, result);
    m_session = new PolkitQt1::Agent::Session(identities.first(), cookie, result);

    connect(m_dialog.data(), &Dialog::accepted, this, &PolKitAgentListener::onDialogAccepted);
    connect(m_dialog.data(), &Dialog::cancel, this, &PolKitAgentListener::onDialogCanceled);

    m_dialog.data()->show();
    m_session.data()->initiate();

    if (!identities.isEmpty()) {
        m_selectedUser = identities[0];
    }

    m_numTries = 0;

    tryAgain();
}

void PolKitAgentListener::request(const QString &request, bool echo)
{
    Q_UNUSED(request);
    Q_UNUSED(echo);
}

void PolKitAgentListener::completed(bool gainedAuthorization)
{
    m_gainedAuthorization = gainedAuthorization;

    finishObtainPrivilege();
}

void PolKitAgentListener::tryAgain()
{
    m_wasCancelled = false;

    if (m_selectedUser.isValid()) {
        m_session = new PolkitQt1::Agent::Session(m_selectedUser, m_cookie, m_result);
        connect(m_session.data(), SIGNAL(request(QString, bool)), this, SLOT(request(QString, bool)));
        connect(m_session.data(), SIGNAL(completed(bool)), this, SLOT(completed(bool)));

        m_session.data()->initiate();
    }
}

void PolKitAgentListener::finishObtainPrivilege()
{
    m_numTries++;

    if (!m_gainedAuthorization && !m_wasCancelled && !m_dialog.isNull()) {
        m_dialog.data()->authenticationFailure();

        if (m_numTries < 3) {
            m_session.data()->deleteLater();

            tryAgain();
            return;
        }
    }

    if (!m_session.isNull()) {
        m_session.data()->result()->setCompleted();
    } else {
        m_result->setCompleted();
    }

    m_session.data()->deleteLater();

    if (!m_dialog.isNull()) {
        m_dialog.data()->deleteLater();
    }

    m_inProgress = false;
}

void PolKitAgentListener::onDialogAccepted()
{
    if (!m_dialog.isNull()) {
        m_session.data()->setResponse(m_dialog.data()->password());
    }
}

void PolKitAgentListener::onDialogCanceled()
{
    m_wasCancelled = true;

    if (!m_session.isNull()) {
        m_session.data()->cancel();
    }

    finishObtainPrivilege();
}
