#include "lastrowtolistproxy.h"

class LastRowToListProxyPrivate : public QObject
{
public:
    QAbstractItemModel* m_pSourceModel {nullptr};
    bool m_UseHeaderDR {false};

    LastRowToListProxy* q_ptr;

public Q_SLOTS:
    void slotRowsInserted();
    void slotLayoutChanged();
};

LastRowToListProxy::LastRowToListProxy(QObject* parent) : QAbstractListModel(parent),
    d_ptr(new LastRowToListProxyPrivate)
{
    d_ptr->q_ptr = this;
}

LastRowToListProxy::~LastRowToListProxy()
{
    delete d_ptr;
}

QModelIndex LastRowToListProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceIndex.isValid()) return {};
    if (!d_ptr->m_pSourceModel) return {};

    const int lastRow = d_ptr->m_pSourceModel->rowCount();

    if (!lastRow) return {};
    if (sourceIndex.row() != lastRow - 1) return {};

    return createIndex(sourceIndex.column(), 0, nullptr);
}

QModelIndex LastRowToListProxy::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!proxyIndex.isValid()) return {};
    if (!d_ptr->m_pSourceModel) return {};

    const unsigned lastRow = d_ptr->m_pSourceModel->rowCount();

    if (!lastRow) return {};

    // This model is a list, so it assumes there is no parent index.
    return d_ptr->m_pSourceModel->index(lastRow-1, proxyIndex.row());
}

QVariant LastRowToListProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if (role == Qt::DisplayRole && d_ptr->m_UseHeaderDR)
        return headerData(idx.row(), Qt::Vertical, Qt::DisplayRole);

    return mapToSource(idx).data(role);
}

int LastRowToListProxy::rowCount(const QModelIndex& parent) const
{
    return ((!parent.isValid()) && d_ptr->m_pSourceModel) ?
        d_ptr->m_pSourceModel->columnCount() : 0;
}

Qt::ItemFlags LastRowToListProxy::flags(const QModelIndex &idx) const
{
    return mapToSource(idx).flags() | Qt::ItemIsDragEnabled;
}

QVariant LastRowToListProxy::headerData(int section, Qt::Orientation o, int role) const
{
    if (!d_ptr->m_pSourceModel) return {};

    return d_ptr->m_pSourceModel->headerData(
        section,
        o == Qt::Horizontal ? Qt::Vertical : Qt::Horizontal,
        role
    );
}

void LastRowToListProxyPrivate::slotRowsInserted()
{
    Q_EMIT q_ptr->dataChanged(q_ptr->index(0,0), q_ptr->index(q_ptr->rowCount(),0));
}

void LastRowToListProxy::setSourceModel(QAbstractItemModel* src)
{
    if (sourceModel()) {
        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsInserted,
            d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
        );

        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsRemoved,
            d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
        );

        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsMoved,
            d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
        );

        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsInserted,
            d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
        );

        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsMoved,
            d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
        );

        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsRemoved,
            d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
        );

        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::layoutChanged,
            d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
        );
    }

    d_ptr->m_pSourceModel = src;

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsInserted,
        d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
    );

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsRemoved,
        d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
    );

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsMoved,
        d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
    );


    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsInserted,
        d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
    );

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsMoved,
        d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
    );

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsRemoved,
        d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
    );

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::layoutChanged,
        d_ptr, &LastRowToListProxyPrivate::slotLayoutChanged
    );

    Q_EMIT layoutChanged();
}


bool LastRowToListProxy::isUsingHeaderAsDisplayRole() const
{
    return d_ptr->m_UseHeaderDR;
}

void LastRowToListProxy::setUsingHeaderAsDisplayRole(bool value)
{
    d_ptr->m_UseHeaderDR = value;
}

QAbstractItemModel* LastRowToListProxy::sourceModel() const
{
    return d_ptr->m_pSourceModel;
}

void LastRowToListProxyPrivate::slotLayoutChanged()
{
    Q_EMIT q_ptr->layoutChanged();
}
