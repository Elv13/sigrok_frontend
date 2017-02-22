#include "mementoproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

struct MementoItem final
{
    int m_Row;
    int m_Column;
    MementoItem* m_pParent;
    QHash<int, QVariant> m_hData;
    QVector<QVector<MementoItem*>> m_llChildren;
};

class MementoProxyPrivate
{
public:
    QVector<QVector<MementoItem*>> m_llRoots {};
    QAbstractItemModel* m_pSourceModel {nullptr};
    QVector<int> m_lRoles;

    void setupExtraRole(const QModelIndex& idx);
    void createSkeleton(const QModelIndex& root, MementoItem* parent);
    void copyData(const QModelIndex& idx, MementoItem* node);
    void clear();
};

MementoProxy::MementoProxy(QObject* parent) : QAbstractItemModel(parent),
d_ptr(new MementoProxyPrivate())
{
}

MementoProxy::MementoProxy(const QJsonObject& json, QObject* parent) : QAbstractItemModel(parent),
d_ptr(new MementoProxyPrivate())
{
    const auto arr = json[QStringLiteral("data")].toArray();

    d_ptr->m_llRoots.resize(arr.size());

    bool firstCell = true;

    for (int row = 0; row < arr.size(); ++row) {
        const auto r = arr[row].toArray();
        d_ptr->m_llRoots[row].resize(r.size());

        for (int col = 0; col < r.size(); ++col) {
            //TODO support trees
            auto node = new MementoItem {
                row, col, nullptr, {}, {}
            };

            const auto cell = r[col].toObject();

            for (auto role = cell.constBegin(); role != cell.constEnd(); ++role) {
                if (firstCell)
                    d_ptr->m_lRoles << role.key().toInt();

                node->m_hData[role.key().toInt()] = role.value().toVariant();
            }

            firstCell = false;

            d_ptr->m_llRoots[row][col] = node;
        }
    }

    Q_EMIT layoutChanged();
}

MementoProxy::~MementoProxy()
{
    while (!d_ptr->m_llRoots.isEmpty()) {
        auto i = d_ptr->m_llRoots.takeLast();
        while (!i.isEmpty())
            delete i.takeLast();
    }
    delete d_ptr;
}

void MementoProxy::setSourceModel(QAbstractItemModel* model)
{
    d_ptr->m_pSourceModel = model;
}

QAbstractItemModel* MementoProxy::sourceModel() const
{
    return d_ptr->m_pSourceModel;
}

QVariant MementoProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto n = static_cast<MementoItem*>(idx.internalPointer());

    return n->m_hData[role];
}

bool MementoProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ((!index.isValid()) || !index.model())
        return false;

    const auto n = static_cast<MementoItem*>(index.internalPointer());

    n->m_hData[role] = value;

    if (role == Qt::EditRole) {
        n->m_hData[Qt::DisplayRole] = value;
        Q_EMIT dataChanged(index, index);
        return true;
    }

    return false;
}

int MementoProxy::rowCount(const QModelIndex& parent) const
{
    if (!d_ptr->m_llRoots.size()) return 0;
    if (!parent.isValid()) return d_ptr->m_llRoots.size();

    const auto n = static_cast<MementoItem*>(parent.internalPointer());
    return n->m_llChildren.size();
}

int MementoProxy::columnCount(const QModelIndex& parent) const
{
    if (!d_ptr->m_llRoots.size()) return 0;

    if (!parent.isValid()) return d_ptr->m_llRoots[0].size();

    const auto n = static_cast<MementoItem*>(parent.internalPointer());
    if (n->m_llChildren.size()) return n->m_llChildren[0].size();

    return 0;
}

QModelIndex MementoProxy::index(int row, int column, const QModelIndex& parent) const
{
    // Root elements
    if (!parent.isValid()) {
        if (row < d_ptr->m_llRoots.size()) {
            if (column < d_ptr->m_llRoots[row].size()) {
                return createIndex(row, column, d_ptr->m_llRoots[row][column]);
            }
        }
    }

    return {};
}

Qt::ItemFlags MementoProxy::flags(const QModelIndex &idx) const
{
    return (!idx.isValid()) ? Qt::NoItemFlags :
        (Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
}


QModelIndex MementoProxy::parent(const QModelIndex& idx) const
{
    if (idx.isValid()) {
        if (auto n = static_cast<MementoItem*>(idx.internalPointer())->m_pParent)
            return createIndex(n->m_Row, n->m_Column, n);
    }

    return {};
}

void MementoProxyPrivate::setupExtraRole(const QModelIndex& idx)
{
    // Check all generic roles
    for (int i=0; i < Qt::UserRole; i++) {
        if (idx.data(i).isValid())
            m_lRoles << i;
    }

    const QHash<int, QByteArray> r = m_pSourceModel->roleNames();

    for (auto i = r.begin(); i != r.end(); ++i) {
        if (idx.data(i.key()).isValid())
            m_lRoles << i.key();
    }
}

void MementoProxyPrivate::copyData(const QModelIndex& idx, MementoItem* node)
{
    const auto roles = m_lRoles; //Avoid detaching the container

    for (const int role : roles) {
        node->m_hData[role] = idx.data(role);
    }
}

void MementoProxyPrivate::createSkeleton(const QModelIndex& root, MementoItem* parent)
{
    const int rc = m_pSourceModel->rowCount(root);
    const int cc = m_pSourceModel->columnCount(root);

    if (parent)
        parent->m_llChildren.resize(rc);
    else
        m_llRoots.resize(rc);

    for (int i=0; i < rc; i++) {

        if (parent)
            parent->m_llChildren[i].resize(cc);
        else
            m_llRoots[i].resize(cc);

        for (int j=0; j < cc; j++) {
            const auto idx = m_pSourceModel->index(i, j, root);
            auto n = new MementoItem {
                i, j, parent, {}, {}
            };

            copyData(idx, n);

            if (parent)
                parent->m_llChildren[i][j] = n;
            else
                m_llRoots[i][j] = n;
        }
    }
}

void MementoProxyPrivate::clear()
{
    const int rc = m_llRoots.size();
    const int cc = rc ? m_llRoots[0].size() : 0;

    for (int i=0; i < rc; i++)
        for (int j=0; j < cc; j++)
            delete m_llRoots[i][j];

    m_llRoots.clear();
}

void MementoProxy::sync()
{
    if (!d_ptr->m_pSourceModel)
        return;

    if (!d_ptr->m_lRoles.size())
        d_ptr->setupExtraRole(d_ptr->m_pSourceModel->index(0,0));

    d_ptr->clear();

    d_ptr->createSkeleton({}, nullptr);

    Q_EMIT layoutChanged();
}

QJsonObject MementoProxy::toJson() const
{
    QJsonObject ret;

    QJsonArray mainArray;

    const int rc = d_ptr->m_llRoots.size();
    const int cc = rc ? d_ptr->m_llRoots[0].size() : 0;
    const auto roles = d_ptr->m_lRoles;

    for (int i=0; i < rc; i++) {
        QJsonArray row;

        for (int j=0; j < cc; j++) {
            QJsonObject cell;

            for (const int role : roles) {
                const auto dt = d_ptr->m_llRoots[i][j]->m_hData[role];
                switch (dt.userType()){
                    case QMetaType::Bool     :
                    case QMetaType::Int      :
                    case QMetaType::UInt     :
                    case QMetaType::Double   :
                    case QMetaType::Long     :
                    case QMetaType::LongLong :
                    case QMetaType::Short    :
                    case QMetaType::Char     :
                    case QMetaType::ULong    :
                    case QMetaType::ULongLong:
                    case QMetaType::UShort   :
                    case QMetaType::SChar    :
                    case QMetaType::UChar    :
                    case QMetaType::Float    :
                        cell[QByteArray::number(role)] = dt.toDouble();
                        break;
                    default:
                        cell[QByteArray::number(role)] = dt.toString();
                        break;
                }
            }

            row.append(cell);
        }

        mainArray.append(row);
    }

    ret[QStringLiteral("data")] = mainArray;

    return ret;
}
