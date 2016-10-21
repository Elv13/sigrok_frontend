#include "proxynodefactory.h"

#include <QtCore/QDebug>

class ProxyNodeFactoryAdapterPrivate
{
public:
    
};

ProxyNodeFactoryAdapter::ProxyNodeFactoryAdapter(GraphicsNodeScene* scene) :
    m_pScene(scene)
{

}

void ProxyNodeFactoryAdapter::registerNode(AbstractNode* o)
{

    QObjectnode* n2 = new QObjectnode(o);
    n2->setTitle(o->title());

    auto l = [o, n2, this]() {

        auto w = o->widget();
        n2->setCentralWidget(w);

        m_pScene->addItem(n2);
        n2->setPos(0,0);
    };
    l();

}


QPair<QObjectnode*, AbstractNode*> ProxyNodeFactoryAdapter::addToScene(const QModelIndex& idx)
{
    auto meta = m_slCategory[idx.parent().row()]->m_lTypes[idx.row()]->m_spMetaObj;

    QObject* o = meta.newInstance();
    Q_ASSERT(o);

    AbstractNode* anode = qobject_cast<AbstractNode*>(o);

    QObjectnode* n2 = new QObjectnode(anode);
    n2->setTitle(anode->title());
    auto w = anode->widget();
    n2->setCentralWidget(w);

    m_pScene->addItem(n2);
    n2->setPos(0,0);

    QPair<QObjectnode*, AbstractNode*> pair {n2, anode};

    m_slCategory[idx.parent().row()]->m_lTypes[idx.row()]->m_lInstances << pair;

    Q_EMIT dataChanged(idx, idx);

    return pair;
}

QVariant ProxyNodeFactoryAdapter::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if (idx.parent().isValid()) {
        const auto cat = m_slCategory[idx.parent().row()];
        switch(role) {
            case Qt::DisplayRole:
                return cat->m_lTypes[idx.row()]->m_Name;
            case Qt::DecorationRole:
                return cat->m_lTypes[idx.row()]->m_Icon;
            case Qt::UserRole:
                return cat->m_lTypes[idx.row()]->m_lInstances.size() > 0 ?
                    cat->m_lTypes[idx.row()]->m_lInstances.size() : QVariant();
        }
    }
    else {
        switch(role) {
            case Qt::DisplayRole:
                return m_slCategory[idx.row()]->m_Name;
        }
    }

    return {};
}

int ProxyNodeFactoryAdapter::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && !parent.parent().isValid())
        return m_slCategory[parent.row()]->m_lTypes.size();
    else if (!parent.isValid())
        return m_slCategory.size();

    return 0;
}

int ProxyNodeFactoryAdapter::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QModelIndex ProxyNodeFactoryAdapter::parent(const QModelIndex& idx) const
{
    if (!idx.isValid()) return {};
    const int id = (int)idx.internalId();
    return id==-1 ? QModelIndex() : index(id, 0);
}

QModelIndex ProxyNodeFactoryAdapter::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column, parent.isValid() ? parent.row() : -1);
}
