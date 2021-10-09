#ifndef HISTORYMODEL_H
#define HISTORYMODEL_H

#include <QAbstractListModel>
#include "notification.h"

class HistoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum Roles {
        IdRole = Qt::UserRole + 1,
        SummaryRole = Qt::DisplayRole,
        ImageRole = Qt::DecorationRole,
        CreatedRole,
        UpdatedRole,
        BodyRole,
        IconNameRole,
        HasDefaultActionRole
    };
    Q_ENUM(Roles)

    static HistoryModel* self();
    explicit HistoryModel(QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE void add(const Notification &notification);
    Q_INVOKABLE void remove(int index);
    Q_INVOKABLE void clearAll();
    Q_INVOKABLE void save();

    void initDatas();

private:
    QVector<Notification> m_notifications;
};

#endif // HISTORYMODEL_H
