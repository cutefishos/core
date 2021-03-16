/*
 * Copyright (C) 2020 PandaOS Team.
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

#include "audiomanager.h"
#include "audioadaptor.h"
#include <QDebug>

AudioManager::AudioManager(QObject *parent)
  : QObject(parent),
    m_engine(new PulseAudioEngine(this)),
    m_defaultSink(nullptr),
    m_defaultSinkIndex(0)
{
    if (m_engine->sinks().count() > 0) {
        m_defaultSink = m_engine->sinks().at(qBound(0, m_defaultSinkIndex, m_engine->sinks().count() - 1));

        new AudioAdaptor(this);
        QDBusConnection::sessionBus().registerObject(QStringLiteral("/Audio"), this);

        connect(m_defaultSink, &AudioDevice::volumeChanged, this, &AudioManager::volumeChanged);
        connect(m_defaultSink, &AudioDevice::muteChanged, this, &AudioManager::muteChanged);
    }
}

int AudioManager::volume()
{
    if (!m_defaultSink)
        return 0;

    return m_defaultSink->volume();
}

bool AudioManager::mute()
{
    if (!m_defaultSink)
        return false;

    return m_defaultSink->mute();
}

void AudioManager::setVolume(int volume)
{
    if (!m_defaultSink)
        return;

    m_defaultSink->setVolume(volume);
}

void AudioManager::toggleMute()
{
    if (!m_defaultSink)
        return;

    m_defaultSink->toggleMute();
}

void AudioManager::setMute(bool state)
{
    if (!m_defaultSink)
        return;

    m_defaultSink->setMute(state);
}
