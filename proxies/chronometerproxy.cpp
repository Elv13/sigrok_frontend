#include "chronometerproxy.h"

#include <QtCore/QDebug>

#include <chrono>

class ChronometerProxyPrivate : public QObject
{
public:
    QVector<quint64> m_lRows;

    int m_ExtraColumnId {0};

    ChronometerProxy* q_ptr;

public Q_SLOTS:
    void slotRowsInserted(const QModelIndex& parent, int start, int end);
    void slotRowsRemoved(const QModelIndex& parent, int start, int end);
    void slotRowsMoved(const QModelIndex& parent, int start, int end);
    void slotClear();
    void slotChangeColumn();
};

ChronometerProxy::ChronometerProxy(QObject* parent) : QIdentityProxyModel(parent),
    d_ptr(new ChronometerProxyPrivate)
{
    d_ptr->q_ptr = this;
}

ChronometerProxy::~ChronometerProxy()
{
    delete d_ptr;
}

int ChronometerProxy::columnCount(const QModelIndex& parent) const
{
    return QIdentityProxyModel::columnCount(parent) + (!parent.isValid() ? 1 : 0);
}

QVariant ChronometerProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if ((!idx.parent().isValid()) && idx.column() == d_ptr->m_ExtraColumnId && role == Qt::DisplayRole) {
        const qint64 delay = (!idx.row()) ? 0 : d_ptr->m_lRows[idx.row()] - d_ptr->m_lRows[idx.row()-1];

        return delay;
    }

    return QIdentityProxyModel::data(idx, role);
}

QModelIndex ChronometerProxy::index(int row, int column, const QModelIndex& parent) const
{
    if (!sourceModel()) return {};

    if ((!parent.isValid()) && column == d_ptr->m_ExtraColumnId && row < sourceModel()->rowCount()) {
        return createIndex(row, column, nullptr);
    }

    return QIdentityProxyModel::index(row, column, parent);
}


Qt::ItemFlags ChronometerProxy::flags(const QModelIndex &idx) const
{
    if (idx.column() == d_ptr->m_ExtraColumnId)
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return QIdentityProxyModel::flags(idx);
}

void ChronometerProxy::setSourceModel(QAbstractItemModel *sm)
{
    QObject::disconnect(this, &QAbstractItemModel::rowsInserted,
        d_ptr, &ChronometerProxyPrivate::slotRowsInserted);
    QObject::disconnect(this, &QAbstractItemModel::rowsRemoved,
        d_ptr, &ChronometerProxyPrivate::slotRowsRemoved);
    QObject::disconnect(this, &QAbstractItemModel::rowsMoved,
        d_ptr, &ChronometerProxyPrivate::slotRowsMoved);
    QObject::disconnect(this, &QAbstractItemModel::layoutChanged,
        d_ptr, &ChronometerProxyPrivate::slotClear);
    QObject::disconnect(this, &QAbstractItemModel::columnsInserted,
        d_ptr, &ChronometerProxyPrivate::slotChangeColumn);
    QObject::disconnect(this, &QAbstractItemModel::columnsRemoved,
        d_ptr, &ChronometerProxyPrivate::slotChangeColumn);

    d_ptr->m_ExtraColumnId = sm->columnCount();

    QIdentityProxyModel::setSourceModel(sm);

    QObject::connect(this, &QAbstractItemModel::rowsInserted,
        d_ptr, &ChronometerProxyPrivate::slotRowsInserted);
    QObject::connect(this, &QAbstractItemModel::rowsRemoved,
        d_ptr, &ChronometerProxyPrivate::slotRowsRemoved);
    QObject::connect(this, &QAbstractItemModel::rowsMoved,
        d_ptr, &ChronometerProxyPrivate::slotRowsMoved);
    QObject::connect(this, &QAbstractItemModel::layoutChanged,
        d_ptr, &ChronometerProxyPrivate::slotClear);
    QObject::connect(this, &QAbstractItemModel::columnsInserted,
        d_ptr, &ChronometerProxyPrivate::slotChangeColumn);
    QObject::connect(this, &QAbstractItemModel::columnsRemoved,
        d_ptr, &ChronometerProxyPrivate::slotChangeColumn);
}


void ChronometerProxyPrivate::slotRowsInserted(const QModelIndex& parent, int start, int end)
{
    if (parent.isValid()) return;

    const quint64 ms = std::chrono::duration_cast< std::chrono::milliseconds >(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();

    m_lRows.resize(q_ptr->sourceModel()->rowCount());

    for (int i = start; i <= end; i++)
        m_lRows[i] = ms;

}

void ChronometerProxyPrivate::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    //FIXME
}

void ChronometerProxyPrivate::slotRowsMoved(const QModelIndex& parent, int start, int end)
{
    //FIXME
}

void ChronometerProxyPrivate::slotClear()
{
    const int rc = q_ptr->sourceModel()->rowCount();
    m_lRows.resize(rc);

    for (int i =0; i < rc;i++)
        m_lRows[i] = 0;
}

void ChronometerProxyPrivate::slotChangeColumn()
{
    m_ExtraColumnId = q_ptr->sourceModel()->columnCount() + 1;
}
