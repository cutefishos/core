#include "historymodel.h"

#include <QSettings>
#include <QDataStream>
#include <QMetaEnum>

static HistoryModel *s_historyModel = nullptr;

HistoryModel *HistoryModel::self()
{
    if (!s_historyModel)
        s_historyModel = new HistoryModel;

    return s_historyModel;
}

HistoryModel::HistoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    initDatas();
}

QVariant HistoryModel::data(const QModelIndex &index, int role) const
{
    const Notification &notification = m_notifications.at(index.row());

    switch (role) {
    case HistoryModel::IdRole:
        return notification.id;
    case HistoryModel::SummaryRole:
        return notification.summary;
    case HistoryModel::ImageRole:
        return "";
    case HistoryModel::CreatedRole:
        return notification.created;
    case HistoryModel::BodyRole:
        return notification.body;
    case HistoryModel::IconNameRole:
        return notification.appIcon;
    case HistoryModel::HasDefaultActionRole:
        return notification.actions.contains("default");
    default:
        break;
    }

    return QVariant();
}

int HistoryModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_notifications.size();
}

QHash<int, QByteArray> HistoryModel::roleNames() const
{
    static QHash<int, QByteArray> s_roles;

    if (s_roles.isEmpty()) {
        // This generates role names from the Roles enum in the form of: FooRole -> foo
        const QMetaEnum e = QMetaEnum::fromType<HistoryModel::Roles>();

        // Qt built-in roles we use
        s_roles.insert(Qt::DisplayRole, QByteArrayLiteral("display"));
        s_roles.insert(Qt::DecorationRole, QByteArrayLiteral("decoration"));

        for (int i = 0; i < e.keyCount(); ++i) {
            const int value = e.value(i);

            QByteArray key(e.key(i));
            key[0] = key[0] + 32; // lower case first letter
            key.chop(4); // strip "Role" suffix

            s_roles.insert(value, key);
        }

        s_roles.insert(HistoryModel::IdRole, QByteArrayLiteral("notificationId")); // id is QML-reserved
    }

    return s_roles;
}

void HistoryModel::add(const Notification &notification)
{
    beginInsertRows(QModelIndex(), 0, 0);
    m_notifications.prepend(std::move(notification));
    endInsertRows();
    save();
}

void HistoryModel::remove(int index)
{
    if (index > m_notifications.size() && index < 0)
        return;

    beginRemoveRows(QModelIndex(), index, index);
    m_notifications.removeAt(index);
    endRemoveRows();
    save();
}

void HistoryModel::clearAll()
{
    beginResetModel();
    m_notifications.clear();
    endResetModel();
    save();
}

void HistoryModel::save()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "notifications");
    settings.clear();

    QByteArray datas;
    QDataStream out(&datas, QIODevice::WriteOnly);

    out << m_notifications;

    settings.setValue("datas", datas);
}

void HistoryModel::initDatas()
{
    QSettings settings(QSettings::UserScope, "cutefishos", "notifications");
    QByteArray listByteArray = settings.value("datas").toByteArray();
    QDataStream in(&listByteArray, QIODevice::ReadOnly);
    in >> m_notifications;
}
