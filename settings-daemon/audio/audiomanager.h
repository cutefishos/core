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

#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include "audio/pulseaudioengine.h"
#include "audio/audiodevice.h"

class AudioManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int volume READ volume WRITE setVolume)
    Q_PROPERTY(bool mute READ mute WRITE setMute)

public:
    AudioManager(QObject *parent = nullptr);

    int volume();
    bool mute();
    void setVolume(int volume);
    void toggleMute();
    void setMute(bool state);

Q_SIGNALS:
    void volumeChanged(int volume);
    void muteChanged(bool state);

private:
    PulseAudioEngine *m_engine;
    AudioDevice *m_defaultSink;
    int m_defaultSinkIndex;
};

#endif
