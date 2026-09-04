#include "clipboard.h"
#include "waylandclipboard.h"

#include <QGuiApplication>
#include <QImage>
#include <QMimeData>
#include <QPixmap>
#include <QVariant>

Clipboard::Clipboard(QObject *parent)
    : QObject(parent)
    , m_qtClipboard(QGuiApplication::clipboard())
{
    if (QGuiApplication::platformName().startsWith(QStringLiteral("wayland"))) {
        m_waylandClipboard = new WlrClipboard(this);
        connect(m_waylandClipboard, &WlrClipboard::dataChanged,
                this, &Clipboard::onDataChanged);
        connect(m_waylandClipboard, &WlrClipboard::availabilityChanged,
                this, &Clipboard::onDataChanged);
    }

    if (m_qtClipboard) {
        connect(m_qtClipboard, &QClipboard::dataChanged,
                this, &Clipboard::onDataChanged);
    }
}

void Clipboard::onDataChanged()
{
    const bool useWaylandClipboard = m_waylandClipboard
            && m_waylandClipboard->isAvailable();
    const QMimeData *mimeData = useWaylandClipboard
            ? m_waylandClipboard->mimeData()
            : (m_qtClipboard ? m_qtClipboard->mimeData(QClipboard::Clipboard) : nullptr);

    if (!mimeData || mimeData->formats().isEmpty())
        return;

    if (mimeData->hasFormat(QStringLiteral("application/x-cutefish-clipboard")) &&
            mimeData->data(QStringLiteral("application/x-cutefish-clipboard")) == QByteArrayLiteral("1"))
        return;

    const QByteArray timeStamp = mimeData->data(QStringLiteral("TIMESTAMP"));

    QMimeData *newMimeData = new QMimeData;
    if (mimeData->hasImage()) {
        const QVariant imageData = mimeData->imageData();
        if (imageData.canConvert<QImage>()) {
            const QImage image = imageData.value<QImage>();
            if (!image.isNull())
                newMimeData->setImageData(image);
        } else if (imageData.canConvert<QPixmap>()) {
            const QPixmap pixmap = imageData.value<QPixmap>();
            if (!pixmap.isNull())
                newMimeData->setImageData(pixmap);
        }
        newMimeData->setData(QStringLiteral("TIMESTAMP"), timeStamp);
    }

    for (const QString &format : mimeData->formats()) {
        if (format == QStringLiteral("image/png")
                || format == QStringLiteral("application/x-qt-image"))
            continue;

        newMimeData->setData(format, mimeData->data(format));
    }

    newMimeData->setData(QStringLiteral("application/x-cutefish-clipboard"), QByteArrayLiteral("1"));

    if (useWaylandClipboard) {
        m_waylandClipboard->setMimeData(newMimeData);
    } else if (m_qtClipboard) {
        m_qtClipboard->setMimeData(newMimeData, QClipboard::Clipboard);
    } else {
        delete newMimeData;
    }
}
