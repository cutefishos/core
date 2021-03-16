#ifndef DIALOG_H
#define DIALOG_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QQuickView>

#include <PolkitQt1/Agent/Listener>

class Dialog : public QObject
{
    Q_OBJECT

public:
    explicit Dialog(const QString &action, const QString &message,
                    const QString &cookie, const QString &identity,
                    PolkitQt1::Agent::AsyncResult *result);
    ~Dialog();

    Q_INVOKABLE void setConfirmationResult(const QString &password = QString());
    Q_INVOKABLE void show();

    Q_PROPERTY(QString message READ message NOTIFY changed)
    Q_PROPERTY(QString action READ action NOTIFY changed)
    Q_PROPERTY(QString cookie READ cookie NOTIFY changed)
    Q_PROPERTY(QString identity READ identity NOTIFY changed)
    Q_PROPERTY(QString password READ password NOTIFY changed)

    QString message() { return m_message; }
    QString action() { return m_action; }
    QString cookie() { return m_cookie; }
    QString identity() { return m_identity; }
    QString password() { return m_password; }

    PolkitQt1::Agent::AsyncResult *result() { return m_result; }

signals:
    // This signal is never emitted at the moment, as we don't change the
    // properties of this window dynamically for now
    void changed();

    // Send approval to the D-Bus helper daemon
    void finished(Dialog *dialog);

private:
    QString m_action;
    QString m_message;
    QVariantMap m_details;
    QString m_cookie;
    QString m_identity;
    QString m_password;
    PolkitQt1::Agent::AsyncResult *m_result;

    QQuickView *m_view;
};

#endif // DIALOG_H
