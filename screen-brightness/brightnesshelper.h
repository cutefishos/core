/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     pjx <pjx206@163.com>
 *             rekols <rekols@foxmail.com>
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

#ifndef BRIGHTNESSHELPER_H
#define BRIGHTNESSHELPER_H

#include <QDir>
#include <QList>
#include <QFile>
#include <QVariantAnimation>
#include <QCoreApplication>
#include <QDebug>
#include <QLockFile>

inline QString readFile(const QString &fileName)
{
    QFile file(fileName);
    if (file.open(QIODevice::ReadOnly)) {
        QString data = QString::fromUtf8(file.readAll());
        file.close();
        return data;
    }

    return QString();
}

inline bool writeFile(const QString &fileName, const QString &data)
{
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream textStream(&file);
        textStream << data;
        textStream.flush();
        file.close();
        return true;
    }

    return false;
}


class BrightnessHelper : public QObject
{
    Q_OBJECT
public:
    explicit BrightnessHelper(QObject *parent = nullptr);
    ~BrightnessHelper();

    void init();

    void setBrightness(int value);
    void increaseBrightness();
    void decreaseBrightness();

    int actual;
    int maxValue;
    QString name;
    QVariantAnimation anime;

private:
    void writeBrightness(int brightness);
};

#endif // BRIGHTNESSHELPER_H
