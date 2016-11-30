#include "columnproxy.h"

#include <QDebug>

class ColumnProxyPrivate : public QObject
{
public:
    explicit ColumnProxyPrivate(ColumnProxy* q) : QObject(q), q_ptr(q) {}

    QAbstractItemModel* m_pSourceModel {nullptr};

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
    }

    d_ptr->m_pSourceModel = source;

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
    qDebug() << (sourceModel() && (!parent.isValid()) ? sourceModel()->columnCount() : 0);
    return sourceModel() && (!parent.isValid()) ? sourceModel()->columnCount() : 0;
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
    Q_UNUSED(idx)
    return {};
}

void ColumnProxyPrivate::slotLayoutChanged()
{
    Q_EMIT q_ptr->layoutChanged();
}

void ColumnProxyPrivate::slotColumnsInserted(const QModelIndex& p, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)
qDebug() << "\n\nCOLUMN PROXY INSERT" << first << last << q_ptr->rowCount();
    if (p.isValid())
        return;

    q_ptr->endInsertRows();

    for (int i=0; i < q_ptr->rowCount();i++)
        qDebug() << i << q_ptr->index(i,0) << q_ptr->index(i,0).data();

    //Q_EMIT q_ptr->dataChanged(q_ptr->index(first,0), q_ptr->index(last, 0));
}

void ColumnProxyPrivate::slotColumnsRemoved(const QModelIndex& p, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)

    if (p.isValid())
        return;

    //FIXME there can be a crash if clicked fast enough
    q_ptr->endRemoveRows();
}

void ColumnProxyPrivate::slotColumnsAboutInserted(const QModelIndex& p, int first, int last)
{
    if (p.isValid())
        return;
qDebug() << "BEGIN INSERT" << first << last;
    q_ptr->beginInsertRows(p, first, last);
}

void ColumnProxyPrivate::slotColumnsAboutRemoved(const QModelIndex& p, int first, int last)
{
    if (p.isValid())
        return;

    q_ptr->beginRemoveRows(p, first, last);
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
