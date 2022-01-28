/*
 * Copyright (C) 2021 CutefishOS Team.
 *
 * Author:     Kate Leet <kate@cutefishos.com>
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

#include "clipboard.h"

#include <QApplication>
#include <QMimeData>
#include <QPixmap>
#include <QDebug>

#include <QBuffer>

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
    , m_qtClipboard(qApp->clipboard())
{
    connect(m_qtClipboard, &QClipboard::dataChanged, this, &Clipboard::onDataChanged);
}

void Clipboard::onDataChanged()
{
    const QMimeData *mimeData = m_qtClipboard->mimeData();

    if (mimeData->formats().isEmpty())
        return;

    if (mimeData->hasFormat("application/x-cutefish-clipboard") &&
            mimeData->data("application/x-cutefish-clipboard") == "1")
        return;

    QByteArray timeStamp = mimeData->data("TIMESTAMP");

    QMimeData *newMimeData = new QMimeData;
    if (mimeData->hasImage()) {
        QPixmap srcPix = m_qtClipboard->pixmap();

        QByteArray bArray;
        QBuffer buffer(&bArray);
        buffer.open(QIODevice::WriteOnly);

        srcPix.save(&buffer);

        newMimeData->setImageData(srcPix);
        newMimeData->setData("TIMESTAMP", timeStamp);
    }

    for (const QString &key : mimeData->formats()) {
        if (key == "image/png" || key == "application/x-qt-image")
            continue;

        newMimeData->setData(key, mimeData->data(key));
    }

    // cutefish flag.
    newMimeData->setData("application/x-cutefish-clipboard", QByteArray("1"));

    m_qtClipboard->setMimeData(newMimeData);
}
