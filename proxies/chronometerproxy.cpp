#include "chronometerproxy.h"

#include <QtCore/QDebug>

#include <chrono>

class ChronometerProxyPrivate : public QObject
{
    Q_OBJECT
public:
    enum class Mode {
        STD_CHRONO,
        ITEM_ROLE,
    };

    QVector<quint64> m_lRows;
    Mode m_Mode       { Mode::STD_CHRONO   };
    bool m_HasRole    { false              };
    int m_EpochRole   { -1                 };
    int m_EpochColumn { -1                 };
    int m_ColCount    {  1                 };
    int m_lCol2Type[5]{ {}, {}, {}, {}, {} };
    int m_ExtraCols   { 0                  };

    ChronometerProxy* q_ptr;

    bool isExtraColumn(const QModelIndex& idx) const;
    ChronometerProxy::ExtraColumns getColumn(const QModelIndex& idx) const;
    ChronometerProxy::ExtraColumns getColumn(int idx, const QModelIndex& parent = {}) const;

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

ChronometerProxy::~ChronometerProxy()
{
    delete d_ptr;
}

bool ChronometerProxyPrivate::isExtraColumn(const QModelIndex& idx) const
{
    return idx.isValid() && idx.internalId()==(quint64)-998;
}

ChronometerProxy::ExtraColumns ChronometerProxyPrivate::getColumn(int idx, const QModelIndex& parent) const
{
    const int baseCol = q_ptr->columnCount(parent) - m_ColCount;

    Q_ASSERT(idx - baseCol >= 0);
    Q_ASSERT(idx - baseCol < m_ColCount);

    return (ChronometerProxy::ExtraColumns) m_lCol2Type[idx - baseCol];
}

ChronometerProxy::ExtraColumns ChronometerProxyPrivate::getColumn(const QModelIndex& idx) const
{
    return getColumn(idx.column(), idx.parent());
}

QModelIndex ChronometerProxy::mapToSource(const QModelIndex& proxyIndex) const
{
    // check if it's the last column without creating an infinite loop
    if (d_ptr->isExtraColumn(proxyIndex))
        return {};

    return QIdentityProxyModel::mapToSource(proxyIndex);
}

int ChronometerProxy::columnCount(const QModelIndex& parent) const
{
    return QIdentityProxyModel::columnCount(parent) + (!parent.isValid() ? d_ptr->m_ColCount : 0);
}

QVariant ChronometerProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if ((!idx.parent().isValid()) && d_ptr->isExtraColumn(idx) && role == Qt::DisplayRole) {
        const auto col = d_ptr->getColumn(idx);

        switch(col) {
            case ExtraColumns::MS_DELTA:
                return (!idx.row()) ?
                    0 : d_ptr->m_lRows[idx.row()] - d_ptr->m_lRows[idx.row()-1];
            case ExtraColumns::MS_EPOCH:
                return 0;
            case ExtraColumns::TIMESTAMP:
                return 0;
            case ExtraColumns::DATE_TIME:
                return 0;
            case ExtraColumns::AUTO:
                break;
        }
    }

    return QIdentityProxyModel::data(idx, role);
}

QModelIndex ChronometerProxy::index(int row, int column, const QModelIndex& parent) const
{
    if (!sourceModel()) return {};

    if ((!parent.isValid()) && column >= columnCount(parent)-d_ptr->m_ColCount) {
        return createIndex(row, column, (quint64) -998);
    }

    return QIdentityProxyModel::index(row, column, parent);
}

QVariant ChronometerProxy::headerData(int s, Qt::Orientation o, int role) const
{
    const int cc = columnCount();
    if (o == Qt::Horizontal && s < cc && s >= cc - d_ptr->m_ColCount && role == Qt::DisplayRole) {
        switch(d_ptr->getColumn(s)) {
            case ExtraColumns::MS_DELTA:
                return QStringLiteral("Delay");
            case ExtraColumns::MS_EPOCH:
                return QStringLiteral("Epoch");
            case ExtraColumns::TIMESTAMP:
                return QStringLiteral("Timestamp");
            case ExtraColumns::DATE_TIME:
                return QStringLiteral("Time");
            case ExtraColumns::AUTO:
                break;
        }
    }

    return QIdentityProxyModel::headerData(s, o, role);
}

Qt::ItemFlags ChronometerProxy::flags(const QModelIndex &idx) const
{
    if (d_ptr->isExtraColumn(idx))
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    return QIdentityProxyModel::flags(idx);
}

void ChronometerProxyPrivate::slotRowsInserted(const QModelIndex& parent, int start, int end)
{
    if (parent.isValid()) return;

    quint64 ms = 0;

    switch(m_Mode) {
        case ChronometerProxyPrivate::Mode::STD_CHRONO:
            ms = std::chrono::duration_cast< std::chrono::milliseconds >(
                std::chrono::system_clock::now().time_since_epoch()
            ).count();
            break;
        case ChronometerProxyPrivate::Mode::ITEM_ROLE:
            ms = (!start) ? 0 : qvariant_cast<quint64>(
                q_ptr->index(start, m_EpochColumn).data(m_EpochRole)
            );
            break;
    }

    m_lRows.resize(q_ptr->sourceModel()->rowCount());

    for (int i = start; i <= end; i++)
        m_lRows[i] = ms;

}

void ChronometerProxyPrivate::slotRowsRemoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
    //FIXME
}

void ChronometerProxyPrivate::slotRowsMoved(const QModelIndex& parent, int start, int end)
{
    Q_UNUSED(parent)
    Q_UNUSED(start)
    Q_UNUSED(end)
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
//     m_ExtraColumnId = q_ptr->sourceModel()->columnCount() + 1;
}

void ChronometerProxy::setTimeSourceColumn(int col)
{
    d_ptr->m_EpochColumn = col;

    if (d_ptr->m_HasRole)
        d_ptr->m_Mode = ChronometerProxyPrivate::Mode::ITEM_ROLE;
}

void ChronometerProxy::setTimeSourceRole(int role)
{
    d_ptr->m_HasRole   = true;
    d_ptr->m_EpochRole = role;

    if (d_ptr->m_EpochColumn != -1)
        d_ptr->m_Mode = ChronometerProxyPrivate::Mode::ITEM_ROLE;
}

void ChronometerProxy::setExtraColumns(ExtraColumns cols)
{
    int       count    = 0;
    const int oldCount = d_ptr->m_ColCount;
    const int cc       = sourceModel() ? sourceModel()->columnCount() : 0;

    // Cache the columns so it doesn't have to be recomputed all the time
    for (const auto c : {
        ExtraColumns::AUTO     , ExtraColumns::MS_DELTA, ExtraColumns::MS_EPOCH,
        ExtraColumns::TIMESTAMP, ExtraColumns::DATE_TIME
    }) {
        if (cols & c) {
            d_ptr->m_lCol2Type[count] = c;
            count++;
        }
    }

    Q_ASSERT(cc+oldCount == columnCount());

    const int newCount = std::max(1, count);

    if (newCount > oldCount)
        beginInsertColumns({}, cc + oldCount, cc + newCount -1);
    else if (newCount < oldCount)
        beginRemoveColumns({}, cc + newCount -1, cc + oldCount -1);

    d_ptr->m_ColCount  = newCount;
    d_ptr->m_ExtraCols = cols;

    if (newCount > oldCount)
        endInsertColumns();
    else if (newCount < oldCount)
        endRemoveColumns();

    Q_ASSERT(cc+newCount == columnCount());
}

int ChronometerProxy::extraColumns() const
{
    return d_ptr->m_ExtraCols;
}

#include <chronometerproxy.moc>
