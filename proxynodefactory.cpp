#include "proxynodefactory.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

#include "qt5-node-editor/src/qnodewidget.h"
#include "qt5-node-editor/src/qreactiveproxymodel.h"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class ProxyNodeFactoryAdapterPrivate
{
public:
    
};

ProxyNodeFactoryAdapter::ProxyNodeFactoryAdapter(QNodeWidget* w) :
    m_pNodeW(w)
{
    m_pNodeW->reactiveModel()->setExtraRole(
        QReactiveProxyModel::ExtraRoles::SourceConnectionNotificationRole, 999
    );
    m_pNodeW->reactiveModel()->setExtraRole(
        QReactiveProxyModel::ExtraRoles::DestinationConnectionNotificationRole, 998
    );
}

QByteArray generateRandomHash()
{
    QByteArray ret(8, '\0');

    for (int i=0; i < 8; i++)
        ret[i] = (char) (qrand() % 42) + '0';

    return ret;
}

void ProxyNodeFactoryAdapter::registerNode(AbstractNode* o)
{

    GraphicsNode* n2;

    switch (o->mode()) {
    case AbstractNode::Mode::PROPERTY:
        n2 = m_pNodeW->addObject(o, o->title(), {}, generateRandomHash());
        break;
    case AbstractNode::Mode::MODEL:
        n2 = m_pNodeW->addModel(o->sourceModel(), o->title(), generateRandomHash());
        break;
    }
    Q_ASSERT(n2 != nullptr);

    n2->setTitle(o->title());

    auto w = o->widget();
    n2->setCentralWidget(w);

    m_pNodeW->scene()->addItem(n2->graphicsItem());
    n2->graphicsItem()->setPos(0,0);

}

QPair<GraphicsNode*, AbstractNode*> ProxyNodeFactoryAdapter::addToSceneFromMetaObject(const QMetaObject& meta, const QString& uid)
{
    QObject* o = meta.newInstance();
    Q_ASSERT(o);
    o->setParent(this);

    AbstractNode* anode = qobject_cast<AbstractNode*>(o);

    GraphicsNode* n2;

    switch (anode->mode()) {
    case AbstractNode::Mode::PROPERTY:
        n2 = m_pNodeW->addObject(anode, anode->title(), {}, uid.isEmpty() ? generateRandomHash() : uid);
        break;
    case AbstractNode::Mode::MODEL:
        n2 = m_pNodeW->addModel(anode->sourceModel(), anode->title(), uid.isEmpty() ? generateRandomHash() : uid);
        break;
    }

    Q_ASSERT(n2 != nullptr);

    n2->setTitle(anode->title());
    auto w = anode->widget();
    n2->setCentralWidget(w);

    m_pNodeW->scene()->addItem(n2->graphicsItem());
    n2->graphicsItem()->setPos(0,0);

    QPair<GraphicsNode*, AbstractNode*> pair {n2, anode};

    m_hIdToType[anode->id()]->m_lInstances << pair;

    return pair;
}

QPair<GraphicsNode*, AbstractNode*> ProxyNodeFactoryAdapter::addToScene(const QModelIndex& idx)
{
    if ((!idx.isValid()) || !idx.parent().isValid())
        return {};

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
    return parent.parent().isValid() ? 0 : 1;
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

    auto sConn = [](QJsonArray& ret, QAbstractItemModel* m, const QString& name) {
        for(int i =0; i < m->rowCount(); i++) {
            const auto edgeI  = m->index(i,1);

            // Only serialize when there is a connection
            if ((!edgeI.data().canConvert<int>()) )
                continue;

            const auto sockI  = m->index(i,0);
            const auto rsockI = m->index(i,2);
            const auto rnode  = m->index(i,3);
            const auto nodeid = rnode.data(Qt::UserRole).toString();

            Q_ASSERT(!nodeid.isEmpty());

            QJsonObject o;

            o[ "direction"    ] = name;
            o[ "own_socket"   ] = sockI.data ().toString();
            o[ "other_socket" ] = rsockI.data().toString();
            o[ "id"           ] = edgeI.data ().toInt();
            o[ "other_node"   ] = nodeid;

            ret.append(o);
        }
    };

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
                const auto sinkModel   = m_pNodeW->sinkSocketModel(nodeW->index());
                const auto sourceModel = m_pNodeW->sourceSocketModel(nodeW->index());

                QJsonObject widget;
                widget["x"    ] = nodeW->graphicsItem()->pos().x();
                widget["y"    ] = nodeW->graphicsItem()->pos().y();
                widget["title"] = nodeW->title();
                widget["UID"  ] = nodeW->index().data(Qt::UserRole).toString();

                QJsonArray conns;
                sConn(conns, sourceModel, QStringLiteral("source"));
                sConn(conns, sinkModel  , QStringLiteral("sink"  ));
                widget["connections"] = conns;

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

    QHash<QString, GraphicsNode*> fromHash;
    QHash<GraphicsNode*, QString> toHash;

    typedef struct {
        int           id;
        bool          isSource;
        GraphicsNode* node;
        QString       ownS;
        QString       otherS;
        QString       otherN;
    } Conn;

    QMap<int, Conn*> connections;

    auto loadConnections = [&connections](const QJsonArray& a, GraphicsNode* s) {
        for (int cId = 0; cId < a.size(); ++cId) {
            const auto c = a[cId].toObject();
            bool isSource = c["direction"] == QLatin1String("source");
            int connId = c["id"].toInt() * (isSource ? 1 : -1);

            const auto nodeId = c["other_node"].toString();

            connections.insert(connId, new Conn {
                /* id;      */ connId,
                /* isSource;*/ isSource,
                /* node;    */ s,
                /* ownS;    */ c["own_socket"  ].toString(),
                /* otherS;  */ c["other_socket"].toString(),
                /* otherN;  */ nodeId,
            });

            Q_ASSERT(nodeId.size()==8 && nodeId == connections[connId]->otherN);
        }
    };

    for (int nodeId = 0; nodeId < nodes.size(); ++nodeId) {
        const QJsonObject node = nodes[nodeId].toObject();

        const auto widget = node[ "widget" ].toObject();
        const auto data   = node[ "data"   ].toObject();

        const QString type = data["id"].toString();

        if (m_hIdToType[type]) {
            const auto uid = widget["UID"].toString();

            Q_ASSERT(uid.size() == 8);

            auto pair = addToSceneFromMetaObject(m_hIdToType[type]->m_spMetaObj, uid);
            pair.second->read(data);

            auto nodeW = pair.first;

            nodeW->graphicsItem()->setPos({
                widget["x"].toInt(),
                widget["y"].toInt()
            });

            if (!widget["title"].toString().isEmpty())
                nodeW->setTitle(widget["title"].toString());

            loadConnections(widget["connections"].toArray(), nodeW);


            if (uid.isEmpty()) {
                qWarning() << "Failed to properly load a node, some links may be missing";
                continue;
            }

            fromHash[uid] = nodeW;
            toHash[nodeW] = uid;

            if (auto m = const_cast<QAbstractItemModel*>(nodeW->index().model()))
                m->setData(nodeW->index(), uid, Qt::UserRole);
        }
    }

    auto edgeM = m_pNodeW->edgeModel();

    // Add the edges
    for (auto i = connections.begin(); i != connections.end(); ++i) {
        const auto conn = i.value();

        if (conn->id >= 0) { // Sources
            // Validate symmetry
            auto otherconn = connections[-i.key()];
            Q_ASSERT(otherconn && otherconn != conn);
            Q_ASSERT(toHash[otherconn->node] == conn->otherN);
            Q_ASSERT(toHash[conn->node ] == otherconn->otherN);

            const int row = edgeM->rowCount() -1;
            const auto srcSock  = conn->node->socketIndex(conn->ownS);
            const auto sinkSock = otherconn->node->socketIndex(otherconn->ownS);

            if ((!srcSock.isValid()) || (!sinkSock.isValid())) {
                qWarning() << "Failed to restore the connection between"
                    << conn->node->title() << ':' << conn->ownS << "and"
                    << otherconn->node->title() << ':' << otherconn->ownS;
                continue;
            }

            edgeM->setData(
                edgeM->index(row, 0),
                srcSock,
                QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
            );
            edgeM->setData(
                edgeM->index(row, 2),
                sinkSock,
                QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
            );

            delete conn;
            delete otherconn;
        }
    }
}

void ProxyNodeFactoryAdapter::load(QIODevice *dev)
{
    const QByteArray data = dev->readAll();
    load(data);
}
