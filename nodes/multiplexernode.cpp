#include "multiplexernode.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

struct CloneHolder
{
    bool                  init     { false                    };
    QString               name     { QStringLiteral("Output") };
    QPersistentModelIndex m_rIndex {                          };
};

class MultiplexerModel final : public QAbstractListModel
{
    Q_OBJECT
    friend class MultiplexerNode;
public:
    explicit MultiplexerModel(QObject* parent = nullptr) :
        QAbstractListModel(parent) {}
    ~MultiplexerModel();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

private:
    int                   m_MaxId {0};
    QVector<CloneHolder*> m_lRows {new CloneHolder};
    QVariant              m_Var;
    QString               m_SinkName {QStringLiteral("Input")};

};

class MultiplexerNodePrivate final
{
public:
    MultiplexerModel* m_pModel {nullptr};
};

QVariant MultiplexerModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if (!idx.row()) {
        switch(role) {
            case Qt::DisplayRole:
                return m_SinkName;
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
                for (auto dh : qAsConst(m_lRows)) {
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
                }
            }
        }

        dh->init = true;

        Q_EMIT dataChanged(idx, idx);

        // Always add more rows when the last once is used
        if (!wasInit) {
            beginInsertRows({}, 1, 1);
            m_lRows.insert(0, new CloneHolder());
            m_lRows.first()->name += QString(" (%1)").arg(m_lRows.size());
            endInsertRows();
        }

        return true;
    }
    else if (idx.isValid() && role == 999) {
        qDebug() << "\n\n\nDISCONNECT";
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
    d_ptr->m_pModel = new MultiplexerModel(this);
}

MultiplexerModel::~MultiplexerModel()
{
    while (!m_lRows.isEmpty())
        delete m_lRows.takeLast();
}

MultiplexerNode::~MultiplexerNode()
{
    delete d_ptr;
}

QString MultiplexerNode::title() const
{
    return QStringLiteral("Multiplexer");
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
    return d_ptr->m_pModel;
}

bool MultiplexerNode::createSinkSocket(const QString& name)
{
    // The name isn't relevant

    d_ptr->m_pModel->m_SinkName = name;

    return true;
}

bool MultiplexerNode::createSourceSocket(const QString& name)
{
    // The name isn't relevant

    auto cl  = new CloneHolder();
    cl->name = name;
    cl->init = true;

    d_ptr->m_pModel->beginInsertRows({}, 1, 1);
    d_ptr->m_pModel->m_lRows.insert(0, cl);
    d_ptr->m_pModel->endInsertRows();

    return true;
}

QStringList MultiplexerNode::searchTags() const
{
    static QStringList l {
        QStringLiteral("divider"),
    };

    return l;
}

#include <multiplexernode.moc>
