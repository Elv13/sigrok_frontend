#include "mementoproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

struct Node final
{
    int m_Row;
    int m_Column;
    Node* m_pParent;
    QHash<int, QVariant> m_hData;
    QVector<QVector<Node*>> m_llChildren;
};

class MementoProxyPrivate
{
public:
    QVector<QVector<Node*>> m_llRoots {};
    QAbstractItemModel* m_pSourceModel {nullptr};
    QVector<int> m_lRoles;

    void setupExtraRole(const QModelIndex& idx);
    void createSkeleton(const QModelIndex& root, Node* parent);
    void copyData(const QModelIndex& idx, Node* node);
    void clear();
};

MementoProxy::MementoProxy(QObject* parent) : QAbstractItemModel(parent),
d_ptr(new MementoProxyPrivate())
{
}

MementoProxy::MementoProxy(const QJsonObject& json, QObject* parent) : QAbstractItemModel(parent),
d_ptr(new MementoProxyPrivate())
{
    const auto arr = json["data"].toArray();

    d_ptr->m_llRoots.resize(arr.size());

    for (int row = 0; row < arr.size(); ++row) {
        const auto r = arr[row].toArray();
        d_ptr->m_llRoots[row].resize(r.size());

        for (int col = 0; col < r.size(); ++col) {
            //TODO support trees
            auto node = new Node {
                row, col, nullptr, {}, {}
            };

            const auto cell = r[col].toObject();

            for (auto role = cell.constBegin(); role != cell.constEnd(); ++role) {
                node->m_hData[role.key().toInt()] = role.value().toVariant();
            }

            d_ptr->m_llRoots[row][col] = node;
        }
    }

    Q_EMIT layoutChanged();
}

MementoProxy::~MementoProxy()
{
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

    const auto n = static_cast<Node*>(idx.internalPointer());

    return n->m_hData[role];
}

int MementoProxy::rowCount(const QModelIndex& parent) const
{
    if (!d_ptr->m_llRoots.size()) return 0;
    if (!parent.isValid()) return d_ptr->m_llRoots.size();

    const auto n = static_cast<Node*>(parent.internalPointer());
    return n->m_llChildren.size();
}

int MementoProxy::columnCount(const QModelIndex& parent) const
{
    if (!d_ptr->m_llRoots.size()) return 0;

    if (!parent.isValid()) return d_ptr->m_llRoots[0].size();

    const auto n = static_cast<Node*>(parent.internalPointer());
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

QModelIndex MementoProxy::parent(const QModelIndex& idx) const
{
    if (idx.isValid()) {
        if (auto n = static_cast<Node*>(idx.internalPointer())->m_pParent)
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

void MementoProxyPrivate::copyData(const QModelIndex& idx, Node* node)
{
    const auto roles = m_lRoles; //Avoid detaching the container

    for (const int role : roles) {
        node->m_hData[role] = idx.data(role);
    }
}

void MementoProxyPrivate::createSkeleton(const QModelIndex& root, Node* parent)
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
            auto n = new Node {
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
                cell[QByteArray::number(role)] = d_ptr->m_llRoots[i][j]->m_hData[role].toString();
            }

            row.append(cell);
        }

        mainArray.append(row);
    }

    ret["data"] = mainArray;

    return ret;
}
