#include "multiplexernode.h"

#include <QtCore/QDebug>

struct CloneHolder
{
    bool                  init     { false                    };
    QString               name     { QStringLiteral("Output") };
    QPersistentModelIndex m_rIndex {                          };
};

class MultiplexerModel final : public QAbstractListModel
{
    friend class MultiplexerNode;
public:

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

private:
    int                   m_MaxId {0};
    QVector<CloneHolder*> m_lRows {new CloneHolder};
    QVariant              m_Var;

};

class MultiplexerNodePrivate final
{
public:
    MultiplexerModel m_Model {};
};

QVariant MultiplexerModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if (!idx.row()) {
        switch(role) {
            case Qt::DisplayRole:
                return QStringLiteral("Input");
            case Qt::EditRole:
                return m_Var;
        }
        return {};
    }

    const auto dh = m_lRows[idx.row()-1];

    switch(role) {
        case Qt::DisplayRole:
            return dh->name;
        case Qt::EditRole:
            return m_Var;
    }

    return {};
}

bool MultiplexerModel::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    if (idx.row() == 0) {
        switch(role) {
            case 999:
            case 998:
                return false;
            default:
                m_Var = value;
                Q_EMIT dataChanged(index(1,0), index(rowCount()-1, 0));

                //FIXME why is this required?
                for (auto dh : m_lRows) {
                    if (auto m = const_cast<QAbstractItemModel*>(dh->m_rIndex.model())) {
                        m->setData(dh->m_rIndex, m_Var, Qt::EditRole);
                    }
                }

                return true;
        }
    }

    if (idx.isValid() && role == 999 && value.canConvert<QModelIndex>()) {
        const int row = idx.row() - 1;

        const auto pidx = value.toPersistentModelIndex();
        auto dh = m_lRows[row];
        dh->m_rIndex = pidx;
        const bool wasInit = dh->init;

        if (pidx.isValid()) {
            if (m_Var.isValid()) {
                if (auto m = const_cast<QAbstractItemModel*>(dh->m_rIndex.model())) {
                    m->setData(dh->m_rIndex, m_Var, Qt::EditRole);
                    return true;
                }
            }
        }

        dh->init = true;

        Q_EMIT dataChanged(idx, idx);

        // Always add more rows when the last once is used
        if ((!wasInit) && row == m_lRows.size() -1) {
            beginInsertRows({}, m_lRows.size(), m_lRows.size());
            m_lRows << new CloneHolder();
            endInsertRows();
        }

        Q_EMIT dataChanged(idx, idx);

        return true;
    }

    return false;
}

int MultiplexerModel::rowCount(const QModelIndex& parent) const
{
    return 1 + (parent.isValid() ? 0 : m_lRows.size());
}

Qt::ItemFlags MultiplexerModel::flags(const QModelIndex &idx) const
{
    if (idx.row() == 0) {
        return Qt::ItemIsEnabled    |
               Qt::ItemIsEditable   |
               Qt::ItemIsSelectable |
               Qt::ItemIsDropEnabled;
    }

    return idx.isValid() ?
        Qt::ItemIsEnabled | Qt::ItemIsSelectable |Qt::ItemIsDragEnabled :
        Qt::NoItemFlags;
}

MultiplexerNode::MultiplexerNode(AbstractSession* sess) : AbstractNode(sess), d_ptr(new MultiplexerNodePrivate())
{
}

MultiplexerNode::~MultiplexerNode()
{
    delete d_ptr;
}

QString MultiplexerNode::title() const
{
    return "Multiplexer";
}

QString MultiplexerNode::id() const
{
    return QStringLiteral("multiplexer_node");
}

void MultiplexerNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* MultiplexerNode::widget() const
{
    return nullptr;
}

QAbstractItemModel* MultiplexerNode::sourceModel() const
{
    return &d_ptr->m_Model;
}

bool MultiplexerNode::createSocket(const QString& name)
{
    // The name isn't relevant

    auto cl  = new CloneHolder();
    cl->name = name;
    cl->init = true;

    const int pos = d_ptr->m_Model.m_lRows.size();
    d_ptr->m_Model.beginInsertRows({}, pos, pos);
    d_ptr->m_Model.m_lRows << cl;
    d_ptr->m_Model.endInsertRows();

    return true;
}
