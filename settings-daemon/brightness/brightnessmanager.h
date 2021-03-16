/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     rekols <rekols@foxmail.com>
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

#ifndef BRIGHTNESSMANAGER_H
#define BRIGHTNESSMANAGER_H

#include <QObject>
#include <QFileSystemWatcher>

class BrightnessManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool brightnessEnabled READ brightnessEnabled)
    Q_PROPERTY(int maxBrightness READ maxBrightness)
    Q_PROPERTY(int brightness READ brightness WRITE setValue)

public:
    BrightnessManager(QObject *parent = nullptr);
    ~BrightnessManager();

    int maxBrightness();
    int brightness();
    bool brightnessEnabled();

    void setValue(int value);

Q_SIGNALS:
    void brightnessChanged();

private:
    void init();
    bool useWhitelistInit();
    void initUsingWhitelist();
    void initUsingBacklightType();

private slots:
    void handleFileChanged(const QString &path);

private:
    QFileSystemWatcher *m_fileWatcher;
    int m_actualBacklight;
    QString m_dirname;
};

#endif
