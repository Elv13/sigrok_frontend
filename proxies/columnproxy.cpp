#include "columnproxy.h"

#include <QDebug>

class ColumnProxyPrivate : public QObject
{
public:
    explicit ColumnProxyPrivate(ColumnProxy* q) : QObject(q), q_ptr(q) {}

    QAbstractItemModel* m_pSourceModel {nullptr};

    ColumnProxy* q_ptr;

public Q_SLOTS:
    void slotColumnChanged();
};

ColumnProxy::ColumnProxy(QObject* parent) :
    QAbstractItemModel(parent), d_ptr(new ColumnProxyPrivate(this))
{
    
}

ColumnProxy::ColumnProxy(QAbstractItemModel* source) :
    QAbstractItemModel(source), d_ptr(new ColumnProxyPrivate(this))
{
    setSourceModel(source);
}

ColumnProxy::~ColumnProxy()
{
    delete d_ptr;
}

void ColumnProxy::setSourceModel(QAbstractItemModel* source)
{
    d_ptr->m_pSourceModel = source;

    //TODO disconnect old

    QObject::connect(source, &QAbstractItemModel::columnsInserted,
                     d_ptr, &ColumnProxyPrivate::slotColumnChanged);

    QObject::connect(source, &QAbstractItemModel::columnsRemoved,
                     d_ptr, &ColumnProxyPrivate::slotColumnChanged);

    QObject::connect(source, &QAbstractItemModel::columnsMoved,
                     d_ptr, &ColumnProxyPrivate::slotColumnChanged);

    Q_EMIT layoutChanged();
}

QAbstractItemModel* ColumnProxy::sourceModel() const
{
    return d_ptr->m_pSourceModel;
}

QVariant ColumnProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    return sourceModel()->headerData(idx.row(), Qt::Horizontal, role);
}

int ColumnProxy::rowCount(const QModelIndex& parent) const
{
    return sourceModel() && !parent.isValid() ? sourceModel()->columnCount() : 0;
}

int ColumnProxy::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 1;
}

QModelIndex ColumnProxy::index(int row, int column, const QModelIndex& parent) const
{
    if (
        (!sourceModel()) ||
        column ||
        parent.isValid() ||
        row >= sourceModel()->columnCount()
    ) return {};

    return createIndex(row, column, nullptr);
}

QModelIndex ColumnProxy::parent(const QModelIndex& idx) const
{
    return {};
}

void ColumnProxyPrivate::slotColumnChanged()
{
    //TODO be more granular
    Q_EMIT q_ptr->layoutChanged();
}

