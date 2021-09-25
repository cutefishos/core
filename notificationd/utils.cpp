#include "utils.h"
#include <QFile>

Utils::Utils(QObject *parent) : QObject(parent)
{

}

QString Utils::processNameFromPid(uint pid)
{
    QFile file(QString("/proc/%1/cmdline").arg(pid));
    QString name;

    if (file.open(QIODevice::ReadOnly)) {
        QByteArray cmd = file.readAll();

        if (!cmd.isEmpty()) {
            // extract non-truncated name from cmdline
            int zeroIndex = cmd.indexOf('\0');
            int processNameStart = cmd.lastIndexOf('/', zeroIndex);
            if (processNameStart == -1) {
                processNameStart = 0;
            } else {
                processNameStart++;
            }
            name = QString::fromLocal8Bit(cmd.mid(processNameStart, zeroIndex - processNameStart));
        }
    }

    return name;
}
