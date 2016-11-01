#include "rangeproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>

#include "rangeproxy_p.h"

QStringListModel* RangeProxyPrivate::s_pDelimiterModel = nullptr;

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

    return ret.toList();
})();

RangeProxy::RangeProxy(QObject* parent) : ColumnProxy(parent),
    d_ptr(new RangeProxyPrivate())
{
    d_ptr->q_ptr = this;
}


int RangeProxy::columnCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return 3;

    const auto node = static_cast<Node*>(parent.internalPointer());

    return node->m_Mode == Node::Mode::CHILD ? 0 : 3;
}

int RangeProxy::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return ColumnProxy::rowCount(parent);

    const auto node = static_cast<Node*>(parent.internalPointer());

    return node->m_lChildren.size();
}

QVariant RangeProxy::headerData(int sec, Qt::Orientation ori, int role) const
{
    return ColumnProxy::headerData(sec, ori, role);
}

QModelIndex RangeProxy::index(int row, int column, const QModelIndex &parent) const
{
    if (row < 0 || column < 0 || column > d_ptr->m_ExtraColumnCount + 1)
        return {};


    if (!parent.isValid() && row < d_ptr->m_lRows.size()) {
        Q_ASSERT(d_ptr->m_lRows[row]);
        return createIndex(row, column, d_ptr->m_lRows[row]);
    }
    else if (parent.isValid()) {
        const auto node = static_cast<Node*>(parent.internalPointer());

        if (row >= node->m_lChildren.size())
            return {};

        Q_ASSERT(node->m_lChildren[row]);
        return createIndex(row, column, node->m_lChildren[row]);
    }

    return {};
}

Qt::ItemFlags RangeProxy::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return Qt::NoItemFlags;

    const auto node = static_cast<Node*>(idx.internalPointer());

    Qt::ItemFlags ret = ColumnProxy::flags(idx);

    return ret | Qt::ItemIsUserCheckable | (
        (node->m_Mode == Node::Mode::CHILD && !idx.column()) ?
            Qt::ItemIsEditable : Qt::NoItemFlags
    );
}

QVariant RangeProxy::data(const QModelIndex& idx, int role) const
{
    if ((!idx.isValid()) || idx.column() > 0) return {};

    const auto node = static_cast<Node*>(idx.internalPointer());

    if (node->m_Mode == Node::Mode::CHILD) {
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
        }
    }

    return ColumnProxy::data(idx, role);
}

QModelIndex RangeProxy::parent(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return {};

    const auto node = static_cast<Node*>(idx.internalPointer());

    if (node->m_Mode == Node::Mode::CHILD)
        return createIndex(node->m_Index, 0, node->m_pParent);

    return {};
}

void RangeProxyPrivate::slotAutoAddRows(const QModelIndex& parent)
{
    if ((!parent.isValid()) || parent.parent().isValid())
        return;

    const auto node = static_cast<Node*>(parent.internalPointer());

    // Each source column need a "NONE" or every row will always match.
    // This is fine for some use case, but breaks many more. Has it hurts,
    // it is done here.
    for (auto n : *const_cast<const QVector<Node*>*>(&node->m_lChildren))
        if (n->m_Delim == RangeProxy::Delimiter::NONE) return;

    const int index = node->m_lChildren.size();

    q_ptr->beginInsertRows(parent, index, index);

    Node* nc      = new Node;
    nc->m_Mode    = Node::Mode::CHILD;
    nc->m_Index   = index;
    nc->m_Delim   = RangeProxy::Delimiter::NONE;
    nc->m_pParent = node;
    node->m_lChildren << nc;

    q_ptr->endInsertRows();
}

bool RangeProxy::setData(const QModelIndex &i, const QVariant &value, int role)
{
    if (!i.isValid())
        return false;

    const auto node = static_cast<Node*>(i.internalPointer());

    if (node->m_Mode != Node::Mode::CHILD || i.column())
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
    d_ptr->slotLayoutChanged();
    Q_EMIT layoutChanged();
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
    if (sourceModel()) {
        disconnect(sourceModel(), &QAbstractItemModel::columnsAboutToBeInserted,
                   d_ptr, &RangeProxyPrivate::slotRowsAboutToBeInserted);
        disconnect(sourceModel(), &QAbstractItemModel::layoutChanged,
                   d_ptr, &RangeProxyPrivate::slotLayoutChanged);
    }

    // It assumes the old and new columns represent the same thing.

    ColumnProxy::setSourceModel(source);

    d_ptr->slotLayoutChanged();

    connect(sourceModel(), &QAbstractItemModel::columnsAboutToBeInserted,
                d_ptr, &RangeProxyPrivate::slotRowsAboutToBeInserted);
    connect(sourceModel(), &QAbstractItemModel::layoutChanged,
                d_ptr, &RangeProxyPrivate::slotLayoutChanged);
}

QModelIndex RangeProxy::matchSourceIndex(const QModelIndex& srcIdx) const
{
    if ((!srcIdx.isValid()) || srcIdx.model() != sourceModel())
        return {};

    if (srcIdx.column() >= d_ptr->m_lRows.size())
        return {};

    const auto colNode = d_ptr->m_lRows[srcIdx.column()];

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
            return createIndex(i, 0, rule);
    }

    return {};
}

void RangeProxyPrivate::slotLayoutChanged()
{
    const int delta = q_ptr->rowCount() - m_lRows.size();

    if (!delta) return;

    slotRowsAboutToBeInserted({}, m_lRows.size(), delta - 1);
}

void RangeProxyPrivate::slotRowsAboutToBeInserted(const QModelIndex &parent, int first, int last)
{
    //FIXME support moved
    const int count = last - first + 1;
    m_lRows.resize(m_lRows.size()+count);

    const bool hasItem = m_lRows.size();

    for (int i = first; i <= last;i++) {
        //Move existing
        if (hasItem && m_lRows.size() > i+count) {
            m_lRows[i+count] = m_lRows[i];
            m_lRows[i+count]->m_Index = i+count;
        }

        // Add the new column
        Node* n = new Node;
        n->m_Index = i;
        m_lRows[i] = n;
    }

    Q_EMIT q_ptr->dataChanged(q_ptr->index(0,0), q_ptr->index(q_ptr->rowCount(),0));

    for (int i = first; i <= last;i++) {
        const auto parent = q_ptr->index(i, 0);
        q_ptr->beginInsertRows(parent, 0,0);
        Node* nc = new Node;
        nc->m_Mode = Node::Mode::CHILD;
        nc->m_pParent = m_lRows[i];
        m_lRows[i]->m_lChildren << nc;
        q_ptr->endInsertRows();
    }
};
