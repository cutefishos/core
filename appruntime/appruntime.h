#ifndef APPRUNTIME_H
#define APPRUNTIME_H

#include <QHash>
#include <QObject>
#include <QStringList>
#include <QTimer>

class ApplicationRegistry;

/**
 * Owns every application start of the session: nothing else in the desktop
 * spawns applications, so the runtime is the single place that knows which
 * application a process belongs to and can therefore quit it again.
 *
 * startProcess() is the only place that actually creates a process, so a
 * booster such as mapplauncherd can be plugged in there alone.
 */
class AppRuntime : public QObject
{
    Q_OBJECT

public:
    explicit AppRuntime(QObject *parent = nullptr);

public slots:
    uint launchApplication(const QString &appId, const QStringList &arguments);
    uint launchCommand(const QString &appId, const QStringList &command,
                       const QString &workingDirectory);
    bool quitApplication(const QString &appId);
    bool quitAll();
    bool quitByPid(uint pid);
    bool isRunning(const QString &appId) const;
    QStringList runningApplications() const;
    QList<uint> pidsForApplication(const QString &appId) const;

signals:
    void applicationLaunched(const QString &appId, uint pid);
    void applicationQuit(const QString &appId, uint pid);

private:
    struct Instance {
        QString appId;
        // Kernel start time of the process; a recycled pid has a different one.
        qulonglong startTime = 0;
    };

    uint startProcess(const QString &appId, const QStringList &command,
                      const QString &workingDirectory);
    bool terminate(uint pid);
    void reap();

    static bool isAlive(uint pid);
    static bool isSafeTarget(uint pid);
    static bool isOwnedByUser(uint pid);
    static qulonglong startTime(uint pid);

    ApplicationRegistry *m_registry;
    // pid -> instance
    QHash<uint, Instance> m_instances;
    QTimer m_reaper;
};

#endif // APPRUNTIME_H
