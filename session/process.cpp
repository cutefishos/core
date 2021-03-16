#include "process.h"

Process::Process(QObject *parent)
    : QProcess(parent)
{
    QProcess::setProcessChannelMode(QProcess::ForwardedChannels);
}

Process::~Process()
{
}
