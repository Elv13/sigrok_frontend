#include "columnproxy.h"

#include <QDebug>

class ColumnProxyPrivate : public QObject
{
public:
    explicit ColumnProxyPrivate(ColumnProxy* q) : QObject(q), q_ptr(q) {}

    QAbstractItemModel* m_pSourceModel {nullptr};
    int m_RowCount {0};

    ColumnProxy* q_ptr;

public Q_SLOTS:
    void slotLayoutChanged();
    void slotColumnsInserted(const QModelIndex& p, int first, int last);
    void slotColumnsRemoved(const QModelIndex& p, int first, int last);
    void slotColumnsAboutInserted(const QModelIndex& p, int first, int last);
    void slotColumnsAboutRemoved(const QModelIndex& p, int first, int last);
//     void slotColumnsMoved(const QModelIndex& p, int first, int last);
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
    if (d_ptr->m_pSourceModel) {
        QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsInserted,
            d_ptr, &ColumnProxyPrivate::slotColumnsInserted);

        QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsRemoved,
            d_ptr, &ColumnProxyPrivate::slotColumnsRemoved);

        QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsAboutToBeInserted,
            d_ptr, &ColumnProxyPrivate::slotColumnsAboutInserted);

        QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved,
            d_ptr, &ColumnProxyPrivate::slotColumnsAboutRemoved);

    //     QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsMoved,
    //                      d_ptr, &ColumnProxyPrivate::slotColumnsMoved);

        QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::layoutChanged,
            d_ptr, &ColumnProxyPrivate::slotLayoutChanged);
        QObject::disconnect(d_ptr->m_pSourceModel, &QAbstractItemModel::modelReset,
            d_ptr, &ColumnProxyPrivate::slotLayoutChanged);
    }

    d_ptr->m_pSourceModel = source;

    if (source) {
        QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsInserted,
                        d_ptr, &ColumnProxyPrivate::slotColumnsInserted);

        QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsRemoved,
                        d_ptr, &ColumnProxyPrivate::slotColumnsRemoved);

        QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsAboutToBeInserted,
                        d_ptr, &ColumnProxyPrivate::slotColumnsAboutInserted);

        QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsAboutToBeRemoved,
                        d_ptr, &ColumnProxyPrivate::slotColumnsAboutRemoved);

    //     QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::columnsMoved,
    //                      d_ptr, &ColumnProxyPrivate::slotColumnsMoved);

        QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::layoutChanged,
                        d_ptr, &ColumnProxyPrivate::slotLayoutChanged);
        QObject::connect(d_ptr->m_pSourceModel, &QAbstractItemModel::modelReset,
                        d_ptr, &ColumnProxyPrivate::slotLayoutChanged);
    }

    d_ptr->slotLayoutChanged();
}

QAbstractItemModel* ColumnProxy::sourceModel() const
{
    return d_ptr->m_pSourceModel;
}

QVariant ColumnProxy::data(const QModelIndex& idx, int role) const
{
    if (!sourceModel()) return {};
    if (!idx.isValid()) return {};

    // Do not forward everything, a buggy sourceModel() headerData will work
    // correctly for the usual views but can cause this to confuse further
    // views or proxies. For example, headerData returning the name without
    // checking the role will break a QComboBox view
    switch (role) {
        case Qt::DisplayRole:
        case Qt::CheckStateRole:
        case Qt::DecorationRole:
        case Qt::BackgroundRole:
        case Qt::ForegroundRole:
            return sourceModel()->headerData(idx.row(), Qt::Horizontal, role);
    }

    return {};
}

int ColumnProxy::rowCount(const QModelIndex& parent) const
{
    // the reason why this is cached rather than just calling sourceModel()
    // columnCount() is because it would just propagate garbage if the
    // sourceModel is buggy and it makes debugging close to impossible.
    return parent.isValid() ? 0 : d_ptr->m_RowCount;
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
        row >= d_ptr->m_RowCount
    ) return {};

    return createIndex(row, column, nullptr);
}

QModelIndex ColumnProxy::parent(const QModelIndex& idx) const
{
    Q_UNUSED(idx)
    return {};
}

void ColumnProxyPrivate::slotLayoutChanged()
{
    m_RowCount = q_ptr->sourceModel() ? q_ptr->sourceModel()->columnCount() : 0;

    Q_EMIT q_ptr->layoutChanged();
}

void ColumnProxyPrivate::slotColumnsInserted(const QModelIndex& p, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (p.isValid() || last > first)
        return;

    q_ptr->endInsertRows();

    //Q_EMIT q_ptr->dataChanged(q_ptr->index(first,0), q_ptr->index(last, 0));
}

void ColumnProxyPrivate::slotColumnsRemoved(const QModelIndex& p, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (p.isValid() || last > first)
        return;

    //FIXME there can be a crash if clicked fast enough
    q_ptr->endRemoveRows();
}

void ColumnProxyPrivate::slotColumnsAboutInserted(const QModelIndex& p, int first, int last)
{
    if (p.isValid() || last > first)
        return;

    q_ptr->beginInsertRows(p, first, last);
    m_RowCount += (last-first)+1;
}

void ColumnProxyPrivate::slotColumnsAboutRemoved(const QModelIndex& p, int first, int last)
{
    if (p.isValid() || last > first)
        return;

    q_ptr->beginRemoveRows(p, first, last);
    m_RowCount -= (last-first)+1;
}

// void ColumnProxyPrivate::slotColumnsMoved(const QModelIndex& p, int first, int last)
// {
//     Q_UNUSED(p)
//     Q_UNUSED(first)
//     Q_UNUSED(last)
//     //TODO
//     if (p.isValid())
//         return;
// 
//     q_ptr->beginMoveRows(p, first, last);
//     q_ptr->endMoveRows();
// }
