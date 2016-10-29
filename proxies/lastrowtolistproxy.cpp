#include "lastrowtolistproxy.h"

class LastRowToListProxyPrivate : public QObject
{
public:
    QAbstractItemModel* m_pSourceModel {nullptr};

    LastRowToListProxy* q_ptr;

public Q_SLOTS:
    void slotRowsInserted();
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

    const unsigned lastRow = d_ptr->m_pSourceModel->rowCount();

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
    return mapToSource(idx).data(role);
}

int LastRowToListProxy::rowCount(const QModelIndex& parent) const
{
    return (
        (!parent.isValid()) && d_ptr->m_pSourceModel && d_ptr->m_pSourceModel->rowCount()
    ) ? 1 : 0;
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
    if (sourceModel())
        disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsInserted,
                   d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
        );

    d_ptr->m_pSourceModel = src;

    connect(d_ptr->m_pSourceModel, &QAbstractItemModel::rowsInserted,
            d_ptr, &LastRowToListProxyPrivate::slotRowsInserted
    );

    Q_EMIT layoutChanged();
}

QAbstractItemModel* LastRowToListProxy::sourceModel() const
{
    return d_ptr->m_pSourceModel;
}
