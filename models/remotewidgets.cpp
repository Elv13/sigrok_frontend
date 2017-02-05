#include "remotewidgets.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#include <QRemoteObjectHost>
#include "common/remotemanager.h"
#include "qt5-node-editor/src/qreactiveproxymodel.h"

struct DataHolder
{
    bool     init {false};
    quint32  m_TypeId {0};
    int      id;
    QPersistentModelIndex m_Index;
    QVariant m_NodeUID;
    QString  name {QStringLiteral("[ADD NEW]")};
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

    RemoteManager::instance()->addModel(clientModel(), roles, QStringLiteral("Remote_controls1"));
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
        case Role::IS_CONNECTED:
            return dh->init;
    }

    return {};
}

bool RemoteWidgets::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.isValid() && role == 999 && value.canConvert<QModelIndex>()) {
        const auto pidx = value.toPersistentModelIndex();
        auto dh = d_ptr->m_lRows[idx.row()];
        const bool wasInit = dh->init;

        dh->init    = true;
        dh->m_Index = pidx;
        dh->name    = dh->m_Index.data().toString();

        Q_EMIT dataChanged(idx, idx);

        // Always add more rows when the last once is used
        if ((!wasInit) && idx.row() == d_ptr->m_lRows.size() -1) {
            beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
            d_ptr->m_lRows << new DataHolder();
            d_ptr->m_lRows.last()->id = d_ptr->m_MaxId++;
            d_ptr->m_lRows.last()->m_NodeUID = pidx.parent().data(Qt::UserRole);
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

RemoteWidgetsClient::RemoteWidgetsClient(RemoteWidgets* src) : QSortFilterProxyModel(src)
{
    setSourceModel(src);
}

bool RemoteWidgetsClient::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    return sourceModel()->index(source_row, 0, source_parent)
        .data(RemoteWidgets::Role::IS_CONNECTED).toBool();
}

QVariant RemoteWidgetsClient::data(const QModelIndex& idx, int role) const
{
    return QSortFilterProxyModel::data(
        idx,
        (role == Qt::DisplayRole || role == Qt::EditRole) ?
            RemoteWidgets::Role::VALUE : role
    );
}

void RemoteWidgets::write(QJsonObject &parent) const
{
    QJsonArray a;

    Q_FOREACH(const auto r, d_ptr->m_lRows) {
        if (r->m_NodeUID.isValid()) {
            QJsonObject ro;
            ro[ QStringLiteral("property") ] = r->name;
            ro[ QStringLiteral("node")     ] = r->m_NodeUID.toString();
            a.append(ro);
        }
    }

    parent[QStringLiteral("properties")] = a;
}

bool RemoteWidgets::addRow(const QString& name)
{
//     if (d_ptr->m_lRows.size() == 1 && !d_ptr->m_lRows[0]->init) {
//         d_ptr->m_lRows[0]->init = true;
//         d_ptr->m_lRows[0]->name = name;
//         return true;
//     }

    auto dh  = new DataHolder;
    dh->name = name;
    dh->init = true;
    dh->id   = d_ptr->m_MaxId++;

    beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
    d_ptr->m_lRows << dh;
    endInsertRows();

    return true;
}

#include <remotewidgets.moc>
