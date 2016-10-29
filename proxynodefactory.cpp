#include "proxynodefactory.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

#include "qt5-node-editor/src/modelnode.hpp"

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

    GraphicsNode* n2;

    switch (o->mode()) {
    case AbstractNode::Mode::PROPERTY:
        n2 = new QObjectnode(o);
        break;
    case AbstractNode::Mode::MODEL:
        n2 = new Modelnode(o->sourceModel());
        break;
    }

    n2->setTitle(o->title());

    auto w = o->widget();
    n2->setCentralWidget(w);

    m_pScene->addItem(n2);
    n2->setPos(0,0);

}

QPair<GraphicsNode*, AbstractNode*> ProxyNodeFactoryAdapter::addToSceneFromMetaObject(const QMetaObject& meta)
{
    QObject* o = meta.newInstance();
    Q_ASSERT(o);

    AbstractNode* anode = qobject_cast<AbstractNode*>(o);

    GraphicsNode* n2;

    switch (anode->mode()) {
    case AbstractNode::Mode::PROPERTY:
        n2 = new QObjectnode(anode);
        break;
    case AbstractNode::Mode::MODEL:
        n2 = new Modelnode(anode->sourceModel());
        break;
    }

    n2->setTitle(anode->title());
    auto w = anode->widget();
    n2->setCentralWidget(w);

    m_pScene->addItem(n2);
    n2->setPos(0,0);

    QPair<GraphicsNode*, AbstractNode*> pair {n2, anode};

    m_hIdToType[anode->id()]->m_lInstances << pair;

    return pair;
}

QPair<GraphicsNode*, AbstractNode*> ProxyNodeFactoryAdapter::addToScene(const QModelIndex& idx)
{
    auto mi = m_slCategory[idx.parent().row()]->m_lTypes[idx.row()];
    auto meta = mi->m_spMetaObj;

    auto ret = addToSceneFromMetaObject(meta);

    Q_EMIT dataChanged(idx, idx);

    return ret;
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


void ProxyNodeFactoryAdapter::serialize(QIODevice *dev) const
{
    QJsonObject session;

    const auto cats = m_slCategory;

    QJsonArray levelArray;

    for (const auto& cat : cats) {
        const auto types = cat->m_lTypes;
        for (const auto& type : types) {
            const auto& elems = type->m_lInstances;
            for (const auto& elem : elems) {
                QJsonObject data;
                elem.second->write(data);

                QJsonObject node;
                node["data"] = data;

                const auto nodeW = elem.first;
                QJsonObject widget;
                widget["x"] = nodeW->pos().x();
                widget["y"] = nodeW->pos().y();
                node["widget"] = widget;

                levelArray.append(node);
            }
        }
    }

    session["nodes"] = levelArray;

    dev->write(QJsonDocument(session).toJson());
}

void ProxyNodeFactoryAdapter::load(const QByteArray& data)
{
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    const auto obj = loadDoc.object();

    const auto nodes = obj["nodes"].toArray();

    for (int nodeId = 0; nodeId < nodes.size(); ++nodeId) {
        const QJsonObject node = nodes[nodeId].toObject();

        const auto widget = node[ "widget" ].toObject();
        const auto data   = node[ "data"   ].toObject();

        const QString type = data["id"].toString();

        if (m_hIdToType[type]) {
            auto pair = addToSceneFromMetaObject(m_hIdToType[type]->m_spMetaObj);
            pair.second->read(data);
            pair.first->setPos({
                widget["x"].toInt(),
                widget["y"].toInt()
            });
        }
    }
}

void ProxyNodeFactoryAdapter::load(QIODevice *dev)
{
    const QByteArray data = dev->readAll();
    load(data);
}
