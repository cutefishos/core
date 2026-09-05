#include "appruntime.h"

#include "applicationregistry.h"
#include "desktopentry.h"

#include <QDebug>
#include <QFileInfo>
#include <QProcess>
#include <QStandardPaths>

#include <errno.h>
#include <signal.h>
#include <unistd.h>

static const int kTerminateTimeout = 5000;

AppRuntime::AppRuntime(QObject *parent)
    : QObject(parent)
    , m_registry(ApplicationRegistry::instance())
{
    // Instances are started detached, so there is no SIGCHLD to wait for.
    m_reaper.setInterval(2000);
    connect(&m_reaper, &QTimer::timeout, this, &AppRuntime::reap);
}

uint AppRuntime::launchApplication(const QString &appId, const QStringList &arguments)
{
    if (appId.isEmpty())
        return 0;

    // Callers know an application either by its entry id or by the path of
    // the desktop file they read.
    DesktopEntry *entry = m_registry->byId(appId);
    if (!entry && appId.contains(QLatin1Char('/'))) {
        entry = m_registry->byPath(appId);

        if (!entry)
            entry = m_registry->byId(QFileInfo(appId).completeBaseName());
    }

    if (!entry) {
        qWarning() << "AppRuntime: no desktop entry for" << appId;
        return 0;
    }

    QStringList command = entry->commandForArguments(arguments);
    if (command.isEmpty())
        return 0;

    if (entry->terminal()) {
        QString terminal = QStandardPaths::findExecutable(QStringLiteral("x-terminal-emulator"));
        if (terminal.isEmpty())
            terminal = QStandardPaths::findExecutable(QStringLiteral("xterm"));

        if (terminal.isEmpty())
            qWarning() << "AppRuntime: no terminal emulator for" << appId;
        else
            command = QStringList{terminal, QStringLiteral("-e")} + command;
    }

    return startProcess(entry->id(), command, entry->workingDirectory());
}

uint AppRuntime::launchCommand(const QString &appId, const QStringList &command,
                               const QString &workingDirectory)
{
    if (command.isEmpty() || command.first().isEmpty())
        return 0;

    QString id = appId;
    if (id.isEmpty())
        id = QFileInfo(command.first()).fileName();

    return startProcess(id, command, workingDirectory);
}

uint AppRuntime::startProcess(const QString &appId, const QStringList &command,
                              const QString &workingDirectory)
{
    QProcess process;
    process.setProgram(command.first());
    process.setArguments(command.mid(1));
    process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    if (!workingDirectory.isEmpty())
        process.setWorkingDirectory(workingDirectory);

    qint64 pid = 0;
    if (!process.startDetached(&pid) || pid <= 0) {
        qWarning() << "AppRuntime: failed to start" << command << process.errorString();
        return 0;
    }

    m_instances.insert(static_cast<uint>(pid), appId);

    if (!m_reaper.isActive())
        m_reaper.start();

    emit applicationLaunched(appId, static_cast<uint>(pid));

    return static_cast<uint>(pid);
}

bool AppRuntime::quitApplication(const QString &appId)
{
    if (appId.isEmpty())
        return false;

    bool result = false;
    for (const uint pid : pidsForApplication(appId))
        result |= terminate(pid);

    return result;
}

// Logout path: one call instead of one per application.
bool AppRuntime::quitAll()
{
    bool result = false;
    for (const uint pid : m_instances.keys())
        result |= terminate(pid);

    return result;
}

bool AppRuntime::quitByPid(uint pid)
{
    return terminate(pid);
}

bool AppRuntime::terminate(uint pid)
{
    if (!isSafeTarget(pid) || !isAlive(pid) || !isOwnedByUser(pid))
        return false;

    // Never signal a process group: Qt's detached children are led by an
    // intermediate fork, so the group is not ours to interpret and a wrong
    // one takes down the whole login session.
    ::kill(static_cast<pid_t>(pid), SIGTERM);

    QTimer::singleShot(kTerminateTimeout, this, [pid] {
        if (isAlive(pid) && isOwnedByUser(pid))
            ::kill(static_cast<pid_t>(pid), SIGKILL);
    });

    if (!m_reaper.isActive())
        m_reaper.start();

    return true;
}

bool AppRuntime::isRunning(const QString &appId) const
{
    return !pidsForApplication(appId).isEmpty();
}

QStringList AppRuntime::runningApplications() const
{
    QStringList result;
    for (auto it = m_instances.constBegin(); it != m_instances.constEnd(); ++it) {
        if (!result.contains(it.value()))
            result.append(it.value());
    }
    return result;
}

QList<uint> AppRuntime::pidsForApplication(const QString &appId) const
{
    QList<uint> result;
    for (auto it = m_instances.constBegin(); it != m_instances.constEnd(); ++it) {
        if (it.value() == appId)
            result.append(it.key());
    }
    return result;
}

void AppRuntime::reap()
{
    const QList<uint> pids = m_instances.keys();
    for (const uint pid : pids) {
        if (isAlive(pid))
            continue;

        const QString appId = m_instances.take(pid);
        emit applicationQuit(appId, pid);
    }

    if (m_instances.isEmpty())
        m_reaper.stop();
}

bool AppRuntime::isAlive(uint pid)
{
    if (pid == 0)
        return false;

    // Detached children are not ours to wait for, so a live pid is never a
    // zombie here.
    return ::kill(static_cast<pid_t>(pid), 0) == 0 || errno == EPERM;
}

bool AppRuntime::isSafeTarget(uint pid)
{
    if (pid <= 1)
        return false;

    // Refuse anything that would take the session down with the application.
    return pid != static_cast<uint>(::getpid())
        && pid != static_cast<uint>(::getpgrp())
        && pid != static_cast<uint>(::getsid(0));
}

bool AppRuntime::isOwnedByUser(uint pid)
{
    const QFileInfo info(QStringLiteral("/proc/%1").arg(pid));
    return info.exists() && info.ownerId() == ::getuid();
}
