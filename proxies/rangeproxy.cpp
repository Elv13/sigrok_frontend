#include "rangeproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>
#include <QtCore/QCoreApplication>
#include <QtCore/QAbstractProxyModel>

#include "rangeproxy_p.h"

QStringListModel* RangeProxyPrivate::s_pDelimiterModel = nullptr;

QHash<QString, int> RangeProxyPrivate::DELIMITERVALUES;

QStringList RangeProxyPrivate::DELIMITERNAMES = ([]() -> QStringList {
    QVector<QString> ret;
    ret.resize((int) RangeProxy::Delimiter::GREATER_EQUAL+1);

    ret[(int) RangeProxy::Delimiter::ANY           ] = QStringLiteral( "Any"  );
    ret[(int) RangeProxy::Delimiter::NONE          ] = QStringLiteral( "None" );
    ret[(int) RangeProxy::Delimiter::EQUAL         ] = QStringLiteral( "=="   );
    ret[(int) RangeProxy::Delimiter::NOT_EQUAL     ] = QStringLiteral( "!="   );
    ret[(int) RangeProxy::Delimiter::LESSER        ] = QStringLiteral( "<"    );
    ret[(int) RangeProxy::Delimiter::GREATER       ] = QStringLiteral( ">"    );
    ret[(int) RangeProxy::Delimiter::LESSER_EQUAL  ] = QStringLiteral( "<="   );
    ret[(int) RangeProxy::Delimiter::GREATER_EQUAL ] = QStringLiteral( ">="   );

    for (int i=0;i < ret.size();i++)
        DELIMITERVALUES[ret[i]] = i;

    return ret.toList();
})();

RangeProxy::RangeProxy(QObject* parent) : ColumnProxy(parent),
    d_ptr(new RangeProxyPrivate())
{
    d_ptr->q_ptr = this;

    connect(this, &QAbstractItemModel::rowsAboutToBeInserted,
            d_ptr, &RangeProxyPrivate::slotRowsAboutToBeInserted);
    connect(this, &QAbstractItemModel::rowsAboutToBeRemoved,
                d_ptr, &RangeProxyPrivate::slotRowsAboutToBeRemoved);
    connect(this, &QAbstractItemModel::layoutChanged,
                d_ptr, &RangeProxyPrivate::slotLayoutChanged);
    connect(this, &QAbstractItemModel::modelReset,
                d_ptr, &RangeProxyPrivate::slotLayoutChanged);
}

RangeProxy::~RangeProxy()
{
    for (auto i : d_ptr->m_lRows) {
        for (auto j : i->m_lChildren)
            delete j;

        delete i;
    }

    delete d_ptr;
}

int RangeProxy::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return 3;

    const auto node = static_cast<RangeProxyNode*>(parent.internalPointer());

    return node->m_Mode == RangeProxyNode::Mode::CHILD ? 0 : 3;
}

int RangeProxy::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return ColumnProxy::rowCount(parent);

    const auto node = static_cast<RangeProxyNode*>(parent.internalPointer());

    return node->m_lChildren.size();
}

QVariant RangeProxy::headerData(int sec, Qt::Orientation ori, int role) const
{
    return ColumnProxy::headerData(sec, ori, role);
}

QModelIndex RangeProxy::index(int row, int column, const QModelIndex &parent) const
{
    if (!sourceModel())
        return {};

    if (row < 0 || column < 0 || column > d_ptr->m_ExtraColumnCount + 1)
        return {};

    if (ColumnProxy::rowCount() != d_ptr->m_lRows.size()) {
        qWarning() << "A proxy is invalid (indexing missing rows)"
            << ColumnProxy::rowCount() << d_ptr->m_lRows.size() << sourceModel();

        d_ptr->slotLayoutChanged();
    }

    if ((!parent.isValid()) && row < d_ptr->m_lRows.size()) {
        Q_ASSERT(d_ptr->m_lRows[row]);
        Q_ASSERT(d_ptr->m_lRows[row]->m_Index == row);
        Q_ASSERT(row == 0 || d_ptr->m_lRows[row] != d_ptr->m_lRows[row] -1);
        return createIndex(row, column, d_ptr->m_lRows[row]);
    }
    else if (parent.isValid()) {
        const auto node = static_cast<RangeProxyNode*>(parent.internalPointer());

        if (row >= node->m_lChildren.size())
            return {};

        Q_ASSERT(row == 0 || node->m_lChildren[row] != node->m_lChildren[row] -1);
        Q_ASSERT(node->m_lChildren[row]->m_pParent == node);
        Q_ASSERT(node->m_lChildren[row]->m_Index == row);

        return createIndex(row, column, node->m_lChildren[row]);
    }

    return {};
}

Qt::ItemFlags RangeProxy::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return Qt::NoItemFlags;

    const auto node = static_cast<RangeProxyNode*>(idx.internalPointer());

    Qt::ItemFlags ret = ColumnProxy::flags(idx);

    return ret | Qt::ItemIsUserCheckable | (
        (node->m_Mode == RangeProxyNode::Mode::CHILD && !idx.column()) ?
            Qt::ItemIsEditable : Qt::NoItemFlags
    );
}

QVariant RangeProxy::data(const QModelIndex& idx, int role) const
{
    if ((!idx.isValid()) || idx.column() > 0) return {};

    const auto node = static_cast<RangeProxyNode*>(idx.internalPointer());

    if (node->m_Mode == RangeProxyNode::Mode::CHILD) {
        switch(role) {
//             case Qt::DisplayRole:
//                 return QString("%1 %2")
//                     .arg(RangeProxyPrivate::DELIMITERNAMES[(int)node->m_Delim])
//                     .arg(node->m_RangeValue.toString());
            case (int) Role::RANGE_DELIMITER:
                return QVariant::fromValue(node->m_Delim);
            case Qt::EditRole:
            case (int) Role::RANGE_VALUE:
                return node->m_RangeValue;
            case (int) Role::RANGE_DELIMITER_NAME:
                return RangeProxyPrivate::DELIMITERNAMES[(int)node->m_Delim];
        }
    }

    return ColumnProxy::data(idx, role);
}

QModelIndex RangeProxy::parent(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return {};

    const auto node = static_cast<RangeProxyNode*>(idx.internalPointer());

    if (node->m_Mode == RangeProxyNode::Mode::CHILD)
        return createIndex(node->m_pParent->m_Index, 0, node->m_pParent);

    return {};
}

void RangeProxyPrivate::slotAutoAddRows(const QModelIndex& parent)
{
    if ((!parent.isValid()) || parent.parent().isValid())
        return;

    const auto node = static_cast<RangeProxyNode*>(parent.internalPointer());

    // Each source column need a "NONE" or every row will always match.
    // This is fine for some use case, but breaks many more. Has it hurts,
    // it is done here.
    for (auto n : *const_cast<const QVector<RangeProxyNode*>*>(&node->m_lChildren))
        if (n->m_Delim == RangeProxy::Delimiter::NONE) return;

    q_ptr->addFilter(parent, RangeProxy::Delimiter::NONE);
}

bool RangeProxy::setData(const QModelIndex &i, const QVariant &value, int role)
{
    if (!i.isValid())
        return false;

    const auto node = static_cast<RangeProxyNode*>(i.internalPointer());

    if (node->m_Mode != RangeProxyNode::Mode::CHILD || i.column())
        return false;

    switch(role) {
        case (int) Role::RANGE_DELIMITER:
            if (value.canConvert<RangeProxy::Delimiter>()) {
                node->m_Delim = qvariant_cast<RangeProxy::Delimiter>(value);
                d_ptr->slotAutoAddRows(i.parent());
                return true;
            }
            else if (value.canConvert<int>()) {
                node->m_Delim = static_cast<RangeProxy::Delimiter>(value.toInt());
                d_ptr->slotAutoAddRows(i.parent());
                return true;
            }
            break;
        case Qt::EditRole:
        case (int) Role::RANGE_VALUE:
            node->m_RangeValue = value;
            d_ptr->slotAutoAddRows(i.parent());
            return true;
    }

    return false;
}

int RangeProxy::extraColumnCount() const
{
    return d_ptr->m_ExtraColumnCount;
}

void RangeProxy::setExtraColumnCount(int value)
{
    d_ptr->m_ExtraColumnCount = value;

    Q_EMIT layoutChanged();
}

bool RangeProxy::matchAllFilters() const
{
    return d_ptr->m_MatchAllColumns;
}

void RangeProxy::setMatchAllFilters(bool value)
{
    d_ptr->m_MatchAllColumns = value;
}

QAbstractItemModel* RangeProxy::delimiterModel() const
{
    if (!d_ptr->s_pDelimiterModel) {
        d_ptr->s_pDelimiterModel = new QStringListModel(
            RangeProxyPrivate::DELIMITERNAMES,
            QCoreApplication::instance()
        );
    }

    return d_ptr->s_pDelimiterModel;
}

void RangeProxy::setSourceModel(QAbstractItemModel* source)
{
    // It assumes the old and new columns represent the same thing.

    ColumnProxy::setSourceModel(source);

    Q_ASSERT(ColumnProxy::rowCount() == d_ptr->m_lRows.size());
}

QModelIndex RangeProxyPrivate::matchSourceIndex(const QModelIndex& srcIdx) const
{
    if ((!srcIdx.isValid()) || srcIdx.model() != q_ptr->sourceModel())
        return {};

    if (srcIdx.column() >= m_lRows.size())
        return {};

    const auto colNode = m_lRows[srcIdx.column()];

    for (int i = 0; i < colNode->m_lChildren.size(); i++) {
        const auto rule = colNode->m_lChildren[i];

        bool match = false;
        // C++14 has a cooler and more functional way to do this, but it ain't
        // supported by all compilers yet
        switch(rule->m_Delim) {
            case RangeProxy::Delimiter::ANY:
                match = true;
                break;
            case RangeProxy::Delimiter::EQUAL:
                match = srcIdx.data() == rule->m_RangeValue;
                break;
            case RangeProxy::Delimiter::NOT_EQUAL:
                match = srcIdx.data() != rule->m_RangeValue;
                break;
            case RangeProxy::Delimiter::LESSER:
                match = srcIdx.data() < rule->m_RangeValue;
                break;
            case RangeProxy::Delimiter::GREATER:
                match = srcIdx.data() > rule->m_RangeValue;
                break;
            case RangeProxy::Delimiter::LESSER_EQUAL:
                match = srcIdx.data() <= rule->m_RangeValue;
                break;
            case RangeProxy::Delimiter::GREATER_EQUAL:
                match = srcIdx.data() >= rule->m_RangeValue;
                break;
            case RangeProxy::Delimiter::NONE:
                break;
        };

        if (match)
            return q_ptr->createIndex(i, 0, rule);
    }

    return {};
}

QModelIndex RangeProxy::matchSourceIndex(const QModelIndex& srcIdx) const
{
    if (d_ptr->m_MatchAllColumns) {
        for (int i = 0; i < rowCount(); i++) {
            auto idx = d_ptr->matchSourceIndex( sourceModel()->index(srcIdx.row(), i));
            if (idx.isValid())
                return idx;
        }
    }

    return d_ptr->matchSourceIndex(srcIdx);
}


QModelIndex RangeProxy::addFilter(const QModelIndex& idx, RangeProxy::Delimiter delim)
{
    if ((!idx.isValid()) || (idx.model() != this))
        return {};

    auto i = idx.parent().isValid() ? idx.parent() : idx;

    const auto node = static_cast<RangeProxyNode*>(i.internalPointer());

    const int idx2 = node->m_lChildren.size();

    beginInsertRows(i, idx2, idx2);

    RangeProxyNode* nc = new RangeProxyNode;
    nc->m_Mode    = RangeProxyNode::Mode::CHILD;
    nc->m_Index   = idx2;
    nc->m_Delim   = delim;
    nc->m_pParent = node;
    node->m_lChildren << nc;

    endInsertRows();

    return index(idx2, 0, idx);
}

QModelIndex RangeProxy::addFilter(const QModelIndex& idx, const QString& delimiter)
{
    Q_ASSERT(d_ptr->DELIMITERVALUES.contains(delimiter));
    return addFilter(idx, (RangeProxy::Delimiter)d_ptr->DELIMITERVALUES[delimiter]);
}

void RangeProxyPrivate::slotLayoutChanged()
{
    const int rc = q_ptr->rowCount();
    const int delta = rc - m_lRows.size();

    if (!delta) return;

    if (m_lRows.size()-1 > rc) {
        const int target = m_lRows.size();
        for(int i=rc; i < target; i++) {
            delete m_lRows[rc];
            m_lRows.remove(rc);
        }
    }

    if (delta > 0)
        slotRowsAboutToBeInserted({}, m_lRows.size(), m_lRows.size() + delta - 1);

    Q_ASSERT(m_lRows.size() == rc);
}

void RangeProxyPrivate::slotRowsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent)

    Q_ASSERT(last >= first);

    //FIXME support moved

    if (parent.isValid())
        return;

    for (int i = first; i <= last;i++) {
        // Add the new column
        RangeProxyNode* n = new RangeProxyNode;
        n->m_Index = i;
        m_lRows.insert(i, n);
    }

    for (int i = last+1; i < m_lRows.size();i++)
        m_lRows[i]->m_Index++;

    //Q_EMIT q_ptr->dataChanged(q_ptr->index(0,0), q_ptr->index(q_ptr->rowCount(),0));
}

void RangeProxyPrivate::slotRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    if (first >= m_lRows.size()) {
        qWarning() << "The proxy is invalid (removing missing rows)";
        return;
    }

    for (int i = first; i <= last;i++) {
        delete m_lRows[i];
        m_lRows.remove(first);
    }

    for (int i = first; i < m_lRows.size();i++)
        m_lRows[i]->m_Index--;
}
