#include "remotewidgets.h"

#include <QtCore/QDebug>

#include <QRemoteObjectHost>
#include "common/remotemanager.h"

#include "remotewidgets_source.h"

struct DataHolder
{
    QString    name {QStringLiteral("[ADD NEW]")};
//     int        role;
    quint32    m_TypeId {0};
    QPersistentModelIndex m_Index;
    int        id;
};

class RemoteWidgetsExport final : public RemoteWidgetsSimpleSource
{
    Q_OBJECT
public:
    explicit RemoteWidgetsExport(QObject* parent = nullptr) : RemoteWidgetsSimpleSource(parent) {}

    RemoteWidgetsPrivate* d_ptr;

public Q_SLOTS:
    void activated(int index, QVariant value);
};

class RemoteWidgetsPrivate
{
public:
    RemoteWidgetsExport m_Exported;
    int m_MaxId {0};
    QVector<DataHolder*> m_lRows {{new DataHolder}};
};

RemoteWidgets::RemoteWidgets(QObject* o) : QAbstractListModel(o),
    d_ptr(new RemoteWidgetsPrivate)
{
    auto host = RemoteManager::instance()->host();
    d_ptr->m_Exported.d_ptr = d_ptr;

    if (!host->enableRemoting(&d_ptr->m_Exported)) {
        qWarning() << "Remote widgets sharing failed";
    }
}

RemoteWidgets::~RemoteWidgets()
{
    
}

void RemoteWidgetsExport::activated(int index, QVariant value)
{

    if (index < d_ptr->m_lRows.size()) {
        auto dh = d_ptr->m_lRows[index];

        if (auto m = const_cast<QAbstractItemModel*>(dh->m_Index.model()))
            m->setData(dh->m_Index, true, Qt::EditRole);
    }
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

        // Add it to the remote control
        auto names = d_ptr->m_Exported.names();
        auto types = d_ptr->m_Exported.types();
        names << d_ptr->m_lRows[idx.row()]->name;
        d_ptr->m_Exported.setNames(names);

        return true;
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

#include <remotewidgets.moc>
