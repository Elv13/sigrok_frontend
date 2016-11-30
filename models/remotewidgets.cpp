#include "remotewidgets.h"

#include <QtCore/QDebug>

#include <QRemoteObjectHost>
#include "common/remotemanager.h"

struct DataHolder
{
    QString    name {QStringLiteral("[ADD NEW]")};
//     int        role;
    quint32    m_TypeId {0};
    QPersistentModelIndex m_Index;
    int        id;
};

class RemoteWidgetsPrivate
{
public:
    mutable RemoteWidgetsClient* m_pClient {Q_NULLPTR};
    int m_MaxId {0};
    QVector<DataHolder*> m_lRows {new DataHolder};
};

RemoteWidgets::RemoteWidgets(QObject* o) : QAbstractListModel(o),
    d_ptr(new RemoteWidgetsPrivate)
{

    static QVector<int> roles {
        Qt::DisplayRole,
        Qt::BackgroundRole
    };

    RemoteManager::instance()->addModel(clientModel(), roles, "Remote_controls1");
}

RemoteWidgets::~RemoteWidgets()
{
    
}

QVariant RemoteWidgets::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    const auto dh = d_ptr->m_lRows[idx.row()];

    switch(role) {
        case Qt::DisplayRole:
            return dh->name;
        case Qt::EditRole:
            return false;
        case Role::VALUE:
            return dh->m_Index.data();
        case Role::MIN_VALUE:
            return 0; //TODO
        case Role::MAX_VALUE:
            return 100; //TODO
        case Role::TYPE_NAME:
            return "bool"; //TODO
    }

    return {};
}

bool RemoteWidgets::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.isValid() && role == 999 && value.canConvert<QModelIndex>()) {
        d_ptr->m_lRows[idx.row()]->m_Index = value.toPersistentModelIndex();

        d_ptr->m_lRows[idx.row()]->name = d_ptr->m_lRows[idx.row()]->m_Index.data().toString();
        Q_EMIT dataChanged(idx, idx);

        // Always add more rows when the last once is used
        if (idx.row() == d_ptr->m_lRows.size() -1) {
            beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
            d_ptr->m_lRows << new DataHolder();
            d_ptr->m_lRows.last()->id = d_ptr->m_MaxId++;
            endInsertRows();
        }

        Q_EMIT dataChanged(idx, idx);

        return true;
    }

    if (idx.isValid() && role == Qt::EditRole) {
        const auto dh = d_ptr->m_lRows[idx.row()];

        if (auto m = const_cast<QAbstractItemModel*>(dh->m_Index.model())) {
            m->setData(dh->m_Index, true, Qt::EditRole);
            return true;
        }

    }

    return false;
}

int RemoteWidgets::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lRows.size();
}

Qt::ItemFlags RemoteWidgets::flags(const QModelIndex &idx) const
{
    return idx.isValid() ?
        Qt::ItemIsEnabled | Qt::ItemIsSelectable |Qt::ItemIsDragEnabled :
        Qt::NoItemFlags;
}

RemoteWidgetsClient* RemoteWidgets::clientModel() const
{
    if (!d_ptr->m_pClient)
        d_ptr->m_pClient = new RemoteWidgetsClient(const_cast<RemoteWidgets*>(this));

    return d_ptr->m_pClient;
}

RemoteWidgetsClient::RemoteWidgetsClient(RemoteWidgets* src) : QIdentityProxyModel(src)
{
    setSourceModel(src);
}

QVariant RemoteWidgetsClient::data(const QModelIndex& idx, int role) const
{
    return QIdentityProxyModel::data(
        idx,
        (role == Qt::DisplayRole || role == Qt::EditRole) ?
            RemoteWidgets::Role::VALUE : role
    );
}

int RemoteWidgetsClient::rowCount(const QModelIndex& parent) const
{
    const int rc = QIdentityProxyModel::rowCount(parent);

    return rc ? rc-1 : 0;
}

#include <remotewidgets.moc>
