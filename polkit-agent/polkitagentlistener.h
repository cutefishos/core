#ifndef POLKITAGENTLISTENER_H
#define POLKITAGENTLISTENER_H

#include <QObject>
#include <QString>

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
    QHash<PolkitQt1::Agent::Session *,PolkitQt1::Identity> m_sessionIdentity;
    Dialog *m_dialog;
};

#endif // POLKITAGENTLISTENER_H
