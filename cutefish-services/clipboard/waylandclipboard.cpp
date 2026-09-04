#include "waylandclipboard.h"

#include "qwayland-wlr-data-control-unstable-v1.h"

#include <QBuffer>
#include <QElapsedTimer>
#include <QGuiApplication>
#include <QImage>
#include <QImageReader>
#include <QImageWriter>
#include <QMetaType>
#include <QMimeData>
#include <QPixmap>
#include <QSet>
#include <QUrl>
#include <QWaylandClientExtensionTemplate>

#include <qguiapplication_platform.h>

#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <unistd.h>

#include <wayland-client-core.h>

#include <functional>
#include <memory>

namespace {

const QString s_textMimeType = QStringLiteral("text/plain");
const QString s_utf8TextMimeType = QStringLiteral("text/plain;charset=utf-8");
const QString s_imageMimeType = QStringLiteral("application/x-qt-image");

bool isImageMimeType(const QString &mimeType)
{
    return mimeType.startsWith(QStringLiteral("image/"));
}

QStringList imageMimeTypes()
{
    QStringList mimeTypes;
    for (const QByteArray &mimeType : QImageWriter::supportedMimeTypes()) {
        const QString value = QString::fromLatin1(mimeType);
        if (isImageMimeType(value) && !mimeTypes.contains(value))
            mimeTypes.append(value);
    }

    if (!mimeTypes.contains(QStringLiteral("image/png")))
        mimeTypes.prepend(QStringLiteral("image/png"));
    else
        mimeTypes.move(mimeTypes.indexOf(QStringLiteral("image/png")), 0);

    return mimeTypes;
}

QByteArray imageFormatForMimeType(const QString &mimeType)
{
    const QList<QByteArray> formats = QImageWriter::imageFormatsForMimeType(mimeType.toLatin1());
    return formats.isEmpty() ? QByteArrayLiteral("PNG") : formats.constFirst();
}

QByteArray imageReadFormatForMimeType(const QString &mimeType)
{
    const QList<QByteArray> formats = QImageReader::imageFormatsForMimeType(mimeType.toLatin1());
    return formats.isEmpty() ? QByteArray() : formats.constFirst();
}

bool readPipe(int fd, QByteArray *data)
{
    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0)
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    pollfd descriptor{fd, POLLIN, 0};
    QElapsedTimer timeout;
    timeout.start();

    while (true) {
        const int remaining = 1000 - static_cast<int>(timeout.elapsed());
        if (remaining <= 0)
            return false;

        const int result = poll(&descriptor, 1, remaining);
        if (result < 0) {
            if (errno == EINTR)
                continue;
            return false;
        }
        if (result == 0)
            return false;

        while (true) {
            char buffer[4096];
            const ssize_t length = read(fd, buffer, sizeof(buffer));
            if (length > 0) {
                data->append(buffer, length);
                continue;
            }
            if (length == 0)
                return true;
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        }

        if (descriptor.revents & (POLLERR | POLLNVAL))
            return false;
        if (descriptor.revents & POLLHUP)
            return true;
        descriptor.revents = 0;
    }
}

bool writePipe(int fd, const QByteArray &data)
{
    struct sigaction ignoreAction{};
    struct sigaction oldAction{};
    ignoreAction.sa_handler = SIG_IGN;
    sigemptyset(&ignoreAction.sa_mask);
    const bool restoreSignal = sigaction(SIGPIPE, &ignoreAction, &oldAction) == 0;

    const char *buffer = data.constData();
    qsizetype remaining = data.size();
    bool success = true;

    const int flags = fcntl(fd, F_GETFL, 0);
    if (flags >= 0 && (flags & O_NONBLOCK))
        fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);

    while (remaining > 0) {
        const ssize_t length = write(fd, buffer, static_cast<size_t>(remaining));
        if (length > 0) {
            buffer += length;
            remaining -= length;
            continue;
        }
        if (length < 0 && errno == EINTR)
            continue;
        success = false;
        break;
    }

    if (restoreSignal)
        sigaction(SIGPIPE, &oldAction, nullptr);
    return success;
}

class WlrDataControlOffer final : public QMimeData,
                                  public QtWayland::zwlr_data_control_offer_v1
{
public:
    explicit WlrDataControlOffer(struct ::zwlr_data_control_offer_v1 *object)
        : QtWayland::zwlr_data_control_offer_v1(object)
    {
    }

    ~WlrDataControlOffer() override
    {
        if (isInitialized())
            destroy();
    }

    QStringList formats() const override
    {
        QStringList result = m_formats;
        if (hasRemoteImage() && !result.contains(s_imageMimeType))
            result.append(s_imageMimeType);
        return result;
    }

    bool hasFormat(const QString &mimeType) const override
    {
        if (mimeType == s_imageMimeType)
            return hasRemoteImage();
        if (mimeType == s_textMimeType && !m_formats.contains(s_textMimeType))
            return m_formats.contains(s_utf8TextMimeType);
        return m_formats.contains(mimeType);
    }

protected:
    void zwlr_data_control_offer_v1_offer(const QString &mimeType) override
    {
        if (!m_formats.contains(mimeType))
            m_formats.append(mimeType);
    }

    QVariant retrieveData(const QString &mimeType, QMetaType preferredType) const override
    {
        if (mimeType == s_imageMimeType && m_data.contains(mimeType))
            return m_data.value(mimeType);

        QString offeredMimeType = mimeType;
        if (mimeType == s_textMimeType && !m_formats.contains(s_textMimeType))
            offeredMimeType = s_utf8TextMimeType;
        else if (mimeType == s_imageMimeType)
            offeredMimeType = remoteImageMimeType();

        if (offeredMimeType.isEmpty())
            return {};

        int pipeFds[2];
        if (pipe(pipeFds) != 0)
            return {};

        const_cast<WlrDataControlOffer *>(this)->receive(offeredMimeType, pipeFds[1]);
        close(pipeFds[1]);

        auto *waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (!waylandApp || !waylandApp->display()) {
            close(pipeFds[0]);
            return {};
        }

        if (wl_display_flush(waylandApp->display()) < 0 && errno != EAGAIN) {
            close(pipeFds[0]);
            return {};
        }

        QByteArray data;
        const bool complete = readPipe(pipeFds[0], &data);
        close(pipeFds[0]);
        if (!complete)
            return {};

        QVariant result;
        if (mimeType == s_imageMimeType) {
            const QByteArray format = imageReadFormatForMimeType(offeredMimeType);
            result = format.isEmpty()
                    ? QVariant(QImage::fromData(data))
                    : QVariant(QImage::fromData(data, format.constData()));
        } else if (mimeType == QStringLiteral("text/uri-list")
                && data.size() > 1
                && preferredType != QMetaType::fromType<QByteArray>()) {
            QList<QUrl> urls;
            for (const QByteArray &encodedUrl : data.split('\n')) {
                const QUrl url = QUrl::fromEncoded(encodedUrl.trimmed());
                if (url.isValid())
                    urls.append(url);
            }
            if (preferredType == QMetaType::fromType<QVariantList>()) {
                QVariantList variantUrls;
                for (const QUrl &url : urls)
                    variantUrls.append(url);
                result = variantUrls;
            } else {
                result = QVariant::fromValue(urls);
            }
        } else if (preferredType == QMetaType::fromType<QString>()) {
            result = QString::fromUtf8(data);
        } else {
            result = data;
        }

        if (mimeType == s_imageMimeType)
            m_data.insert(mimeType, result);
        return result;
    }

private:
    bool hasRemoteImage() const
    {
        return m_formats.contains(s_imageMimeType) || !remoteImageMimeType().isEmpty();
    }

    QString remoteImageMimeType() const
    {
        if (m_formats.contains(s_imageMimeType))
            return s_imageMimeType;
        for (const QString &mimeType : m_formats) {
            if (isImageMimeType(mimeType)
                    && !QImageReader::imageFormatsForMimeType(mimeType.toLatin1()).isEmpty())
                return mimeType;
        }
        return {};
    }

    QStringList m_formats;
    mutable QHash<QString, QVariant> m_data;
};

class WlrDataControlSource final : public QObject,
                                   public QtWayland::zwlr_data_control_source_v1
{
public:
    WlrDataControlSource(struct ::zwlr_data_control_source_v1 *object,
                         std::unique_ptr<QMimeData> mimeData,
                         std::function<void()> cancelled)
        : QtWayland::zwlr_data_control_source_v1(object)
        , m_mimeData(std::move(mimeData))
        , m_cancelled(std::move(cancelled))
    {
        QSet<QString> offeredFormats;
        for (const QString &mimeType : m_mimeData->formats()) {
            offer(mimeType);
            offeredFormats.insert(mimeType);
        }

        if (m_mimeData->hasText() && !offeredFormats.contains(s_utf8TextMimeType))
            offer(s_utf8TextMimeType);

        if (m_mimeData->hasImage()) {
            for (const QString &mimeType : imageMimeTypes()) {
                if (!offeredFormats.contains(mimeType))
                    offer(mimeType);
            }
        }
    }

    ~WlrDataControlSource() override
    {
        if (isInitialized())
            destroy();
    }

    const QMimeData *mimeData() const
    {
        return m_mimeData.get();
    }

protected:
    void zwlr_data_control_source_v1_send(const QString &mimeType, int32_t fd) override
    {
        QByteArray data;
        if (m_mimeData->hasImage() && (mimeType == s_imageMimeType || isImageMimeType(mimeType))) {
            const QVariant imageData = m_mimeData->imageData();
            QImage image;
            if (imageData.canConvert<QImage>())
                image = imageData.value<QImage>();
            else if (imageData.canConvert<QPixmap>())
                image = imageData.value<QPixmap>().toImage();

            if (!image.isNull()) {
                QBuffer buffer(&data);
                buffer.open(QIODevice::WriteOnly);
                image.save(&buffer, imageFormatForMimeType(mimeType).constData());
            }
        } else {
            QString requestedMimeType = mimeType;
            if (mimeType == s_utf8TextMimeType && !m_mimeData->hasFormat(mimeType))
                requestedMimeType = s_textMimeType;
            data = m_mimeData->data(requestedMimeType);
        }

        writePipe(fd, data);
        close(fd);
    }

    void zwlr_data_control_source_v1_cancelled() override
    {
        if (m_cancelled)
            m_cancelled();
    }

private:
    std::unique_ptr<QMimeData> m_mimeData;
    std::function<void()> m_cancelled;
};

class WlrDataControlManager;

class WlrDataControlDevice final : public QObject,
                                   public QtWayland::zwlr_data_control_device_v1
{
public:
    WlrDataControlDevice(struct ::zwlr_data_control_device_v1 *object,
                          WlrDataControlManager *manager,
                          std::function<void()> selectionChanged)
        : QtWayland::zwlr_data_control_device_v1(object)
        , m_manager(manager)
        , m_selectionChanged(std::move(selectionChanged))
    {
    }

    ~WlrDataControlDevice() override
    {
        m_source.reset();
        m_selection.reset();
        m_pendingOffer.reset();
        if (isInitialized())
            destroy();
    }

    const QMimeData *mimeData() const
    {
        return m_source ? m_source->mimeData() : m_selection.get();
    }

    void setMimeData(std::unique_ptr<QMimeData> mimeData);

protected:
    void zwlr_data_control_device_v1_data_offer(struct ::zwlr_data_control_offer_v1 *object) override
    {
        m_pendingOffer = std::make_unique<WlrDataControlOffer>(object);
    }

    void zwlr_data_control_device_v1_selection(struct ::zwlr_data_control_offer_v1 *object) override
    {
        if (!object) {
            m_selection.reset();
        } else {
            auto *baseOffer = QtWayland::zwlr_data_control_offer_v1::fromObject(object);
            auto *offer = dynamic_cast<WlrDataControlOffer *>(baseOffer);
            if (offer != m_pendingOffer.get())
                return;
            m_selection = std::move(m_pendingOffer);
        }

        if (m_selectionChanged)
            m_selectionChanged();
    }

    void zwlr_data_control_device_v1_primary_selection(struct ::zwlr_data_control_offer_v1 *) override
    {
        m_pendingOffer.reset();
    }

    void zwlr_data_control_device_v1_finished() override
    {
        if (isInitialized())
            destroy();
    }

private:
    WlrDataControlManager *m_manager;
    std::function<void()> m_selectionChanged;
    std::unique_ptr<WlrDataControlSource> m_source;
    std::unique_ptr<WlrDataControlOffer> m_selection;
    std::unique_ptr<WlrDataControlOffer> m_pendingOffer;
};

class WlrDataControlManager final
    : public QWaylandClientExtensionTemplate<WlrDataControlManager>
    , public QtWayland::zwlr_data_control_manager_v1
{
public:
    WlrDataControlManager()
        : QWaylandClientExtensionTemplate<WlrDataControlManager>(2)
    {
    }

    ~WlrDataControlManager()
    {
        if (isInitialized())
            destroy();
    }

    void start()
    {
        initialize();
    }
};

void WlrDataControlDevice::setMimeData(std::unique_ptr<QMimeData> mimeData)
{
    if (!m_manager || !m_manager->isActive())
        return;

    if (!mimeData)
        return;

    struct ::zwlr_data_control_source_v1 *object = m_manager->create_data_source();
    if (!object)
        return;

    auto source = std::make_unique<WlrDataControlSource>(
            object, std::move(mimeData), [this] {
                QMetaObject::invokeMethod(this, [this] {
                    m_source.reset();
                    if (m_selectionChanged)
                        m_selectionChanged();
                }, Qt::QueuedConnection);
            });

    set_selection(source->object());
    m_source = std::move(source);
}

} // namespace

class WlrClipboard::Private : public QObject
{
public:
    explicit Private(WlrClipboard *q)
        : QObject(q)
        , q(q)
    {
        QObject::connect(&manager, &QWaylandClientExtension::activeChanged,
                         this, [this] {
            if (!manager.isActive()) {
                device.reset();
                emit this->q->availabilityChanged();
                return;
            }

            auto *waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
            if (!waylandApp || !waylandApp->seat())
                return;

            auto *object = manager.get_data_device(waylandApp->seat());
            if (!object)
                return;

            device = std::make_unique<WlrDataControlDevice>(object, &manager, [this] {
                emit this->q->dataChanged();
            });
            emit this->q->availabilityChanged();
        });
    }

    WlrClipboard *q;
    WlrDataControlManager manager;
    std::unique_ptr<WlrDataControlDevice> device;
};

WlrClipboard::WlrClipboard(QObject *parent)
    : QObject(parent)
    , d(new Private(this))
{
    QMetaObject::invokeMethod(d, [this] {
        auto *waylandApp = qGuiApp->nativeInterface<QNativeInterface::QWaylandApplication>();
        if (!waylandApp || !waylandApp->display() || !waylandApp->seat())
            return;
        d->manager.start();
    }, Qt::QueuedConnection);
}

WlrClipboard::~WlrClipboard()
{
    delete d;
}

bool WlrClipboard::isAvailable() const
{
    return d->device != nullptr;
}

const QMimeData *WlrClipboard::mimeData() const
{
    return d->device ? d->device->mimeData() : nullptr;
}

void WlrClipboard::setMimeData(QMimeData *mimeData)
{
    if (!d->device) {
        delete mimeData;
        return;
    }
    d->device->setMimeData(std::unique_ptr<QMimeData>(mimeData));
}
