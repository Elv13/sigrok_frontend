#include "session.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtCore/QTimer>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QItemSelectionModel>

#include "qt5-node-editor/src/qnodewidget.h"
#include "qt5-node-editor/src/qreactiveproxymodel.h"

#include "common/interfaceserializer.h"
#include "common/pagemanager.h"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class NodeMimeData : public QMimeData
{
    Q_OBJECT
public:
    explicit NodeMimeData(Session* f, const QModelIndex& idx) :
    QMimeData(), m_pParent(f), m_Index(idx) {}

    virtual bool hasFormat(const QString &mimeType) const override {
        return mimeType == QStringLiteral("x-qnodeview/node-index");
    }

protected:
    virtual QVariant retrieveData(const QString &mt, QVariant::Type t) const override
    {
        Q_UNUSED(t)

        if (mt != QLatin1String("x-qnodeview/node-index"))
            return {};

        Q_ASSERT(m_Index.isValid());
        Q_ASSERT(m_Index.parent().isValid());

        auto node = m_pParent->addToScene(m_Index);

        const auto  rect = node.first->rect();
        const float x    = property("x-qnodeview/position-x").toFloat();
        const float y    = property("x-qnodeview/position-y").toFloat();

        node.first->graphicsItem()->setPos(x-rect.width()/2,y-rect.height()/2);

        m_pParent->m_pNodeW->setCurrentNode(node.first);

        return {};
    }

    const QPersistentModelIndex m_Index;
    Session* m_pParent;
};

class SessionPrivate
{
public:
    QList<InterfaceSerializer*> m_lIS;
    PageManager m_PageManager;
};

Session::Session(QNodeWidget* w) : AbstractSession(w),
    m_pNodeW(w), d_ptr(new SessionPrivate)
{
    m_pNodeW->reactiveModel()->setExtraRole(
        QReactiveProxyModel::ExtraRoles::SourceConnectionNotificationRole, 999
    );
    m_pNodeW->reactiveModel()->setExtraRole(
        QReactiveProxyModel::ExtraRoles::DestinationConnectionNotificationRole, 998
    );
    connect(w, SIGNAL(nodeRenamed(QString, QString, QString)), this, SLOT(renameN(QString, QString)));
}

PageManager* Session::pages() const
{
    return &d_ptr->m_PageManager;
}

void Session::registerInterfaceSerializer(InterfaceSerializer* ser)
{
    d_ptr->m_lIS << ser;
}

// Ensure that UIDs are unique
static QHash<QString, bool> g_Used;

static QByteArray generateRandomHash();
QByteArray generateRandomHash()
{
    QByteArray ret(8, '\0');

    for (int i=0; i < 8; i++)
        ret[i] = (char) (qrand() % 42) + '0';

    // Avoids duplicate
    if (g_Used.contains(ret))
        return generateRandomHash();

    g_Used[ret] = true;

    return ret;
}

Session::NodePair Session::addToSceneFromMetaObject(const QMetaObject& meta, const QString& uid)
{
    QObject* o = meta.newInstance(Q_ARG(AbstractSession*, this));
    Q_ASSERT(o);

    AbstractNode* anode = qobject_cast<AbstractNode*>(o);

    const auto uid2 = uid.isEmpty() ? generateRandomHash() : uid;

    anode->setUid(uid2);

    GraphicsNode* n2;

    switch (anode->mode()) {
    case AbstractNode::Mode::PROPERTY:
        n2 = m_pNodeW->addObject(anode, anode->title(), {}, uid2);
        anode->setParent(m_pNodeW);
        break;
    case AbstractNode::Mode::MODEL:
        n2 = m_pNodeW->addModel(anode->sourceModel(), anode->title(), uid2);
        anode->setParent(anode->sourceModel());
        anode->sourceModel()->setParent(m_pNodeW);
        break;
    }

    Q_ASSERT(n2 != nullptr);

    n2->setTitle(anode->title());

    // Make sure "read" is called before the widget is created. This also
    // ensure the connections are already done
    QTimer::singleShot(0, [anode, n2]() {
        auto w = anode->widget();
        n2->setCentralWidget(w);
    });

    n2->graphicsItem()->setPos(0,0);
    n2->setProperty("uid", uid2);

    NodePair pair {n2, anode};

    const auto id = anode->id();

    m_hIdToType[id]->m_lInstances << pair;
    m_hIdToNode[uid2] = pair;

    connect(anode, &QObject::destroyed, [anode, id, this]() {
        remove(anode, id);
    });

    return pair;
}

Session::NodePair Session::addToScene(const QModelIndex& idx)
{
    if ((!idx.isValid()) || !idx.parent().isValid())
        return {};

    auto mi   = m_slCategory[idx.parent().row()]->m_lTypes[idx.row()];
    auto meta = mi->m_spMetaObj;

    auto ret = addToSceneFromMetaObject(meta);

    ret.first->setDecoration(mi->m_Icon);

    Q_EMIT dataChanged(idx, idx);

    return ret;
}

void Session::remove(const QObject* n, const QString& id)
{
    Q_ASSERT(n);

    int idx = -1;

    auto i = m_hIdToType[id];

    const auto& l = i->m_lInstances;

    for (int j =0; j < l.size(); j++) {
        if (l[j].second == n) {
            m_hIdToNode.remove(l[j].second->uid());
            idx = j;
            break;
        }
    }

    if (idx != -1) {
        i->m_lInstances.remove(idx);
    }

    const auto midx = createIndex(i->m_Index, 0, -1 );

    Q_EMIT dataChanged(midx, midx);
}

void Session::renameN(const QString& uid, const QString& name)
{
    for (const auto is : qAsConst(d_ptr->m_lIS))
        is->rename(uid, name);
}

QVariant Session::data(const QModelIndex& idx, int role) const
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

int Session::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid() && !parent.parent().isValid())
        return m_slCategory[parent.row()]->m_lTypes.size();
    else if (!parent.isValid())
        return m_slCategory.size();

    return 0;
}

int Session::columnCount(const QModelIndex& parent) const
{
    return parent.parent().isValid() ? 0 : 1;
}

QModelIndex Session::parent(const QModelIndex& idx) const
{
    if (!idx.isValid()) return {};
    const int id = (int)idx.internalId();
    return id==-1 ? QModelIndex() : index(id, 0);
}

Qt::ItemFlags Session::flags(const QModelIndex& idx) const
{
    if (idx.parent().isValid())
        return QAbstractItemModel::flags(idx) | Qt::ItemIsDragEnabled;

    return QAbstractItemModel::flags(idx);
}

QStringList Session::mimeTypes() const
{
    return {QStringLiteral("x-qnodeview/node-index")};
}

QMimeData* Session::mimeData(const QModelIndexList& indexes) const
{
    if (indexes.size() != 1)
        return nullptr;

    const auto idx = indexes.first();

    // Categories can't be dragged
    if (!idx.parent().isValid())
        return nullptr;

    auto dt = new NodeMimeData(const_cast<Session*>(this), idx);

    return dt;
}

QModelIndex Session::index(int row, int column, const QModelIndex& parent) const
{
    return createIndex(row, column, parent.isValid() ? parent.row() : -1);
}

void Session::serialize(QIODevice *dev) const
{
    QJsonObject session;

    const auto cats = m_slCategory;

    auto sConn = [](QJsonArray& ret, QAbstractItemModel* m, const QString& name) {
        QHash<QString, bool> singleSink;

        for(int i =0; i < m->rowCount(); i++) {
            const auto edgeI  = m->index(i,1);


            // Only serialize when there is a connection
            if ((!edgeI.data().canConvert<int>()) )
                continue;

            const auto sockI  = m->index(i,0);
            const auto rsockI = m->index(i,2);
            const auto rnode  = m->index(i,3);

            Q_ASSERT(sockI.isValid());
            Q_ASSERT(rsockI.isValid());
            Q_ASSERT(rnode.isValid());

            const auto nodeid = rnode.data(Qt::UserRole).toString();

            Q_ASSERT(!nodeid.isEmpty());

            QJsonObject o;
            const auto socketName = sockI.data ().toString();

            // Make sure a socket has only 1 edge
            if (singleSink.contains(socketName)) {
                qWarning() << "Trying to save 2 edges for the same socket, aborting"
                    << socketName << rsockI.data().toString();

                Q_ASSERT(!singleSink.contains(socketName));
                continue;
            }

            o[ QStringLiteral("direction")    ] = name;
            o[ QStringLiteral("own_socket")   ] = socketName;
            o[ QStringLiteral("other_socket") ] = rsockI.data().toString();
            o[ QStringLiteral("id")           ] = edgeI.data ().toInt   ();
            o[ QStringLiteral("other_node")   ] = nodeid;

            singleSink[socketName] = true;

            ret.append(o);
        }
    };

    QJsonArray levelArray;

    for (const auto& cat : qAsConst(cats)) {
        const auto types = cat->m_lTypes;
        for (const auto& type : qAsConst(types)) {
            const auto& nodes = type->m_lInstances;
            for (const auto& nodeJ : qAsConst(nodes)) {
                QJsonObject data;
                nodeJ.second->write(data);

                QJsonObject node;
                node[QStringLiteral("data")] = data;

                const auto nodeW       = nodeJ.first;
                const auto sinkModel   = m_pNodeW->sinkSocketModel(nodeW->index());
                const auto sourceModel = m_pNodeW->sourceSocketModel(nodeW->index());

                const QRectF r = nodeW->rect();

                QJsonObject widget;
                widget[QStringLiteral("x")     ] = r.x();
                widget[QStringLiteral("y")     ] = r.y();
                widget[QStringLiteral("width") ] = r.width();
                widget[QStringLiteral("height")] = r.height();
                widget[QStringLiteral("title") ] = nodeW->title();
                widget[QStringLiteral("UID")   ] = nodeW->index().data(Qt::UserRole).toString();

                QJsonArray conns;
                sConn(conns, sourceModel, QStringLiteral("source"));
                sConn(conns, sinkModel  , QStringLiteral("sink"  ));
                widget[QStringLiteral("connections")] = conns;

                node[QStringLiteral("widget")] = widget;

                levelArray.append(node);
            }
        }
    }

    session[QStringLiteral("nodes")] = levelArray;

    QJsonArray views;

    for (const auto is : qAsConst(d_ptr->m_lIS)) {
        QJsonObject view;
        is->write(view);
        views.append(view);
    }

    session[QStringLiteral("views")] = views;

    dev->write(QJsonDocument(session).toJson());
}

void Session::load(const QByteArray& data)
{
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    const auto obj = loadDoc.object();

    const auto nodes = obj[QStringLiteral("nodes")].toArray();

    QHash<GraphicsNode*, QString> toHash;
    QHash<GraphicsNode*, AbstractNode*> toNode;

    typedef struct {
        int           id;
        bool          isSource;
        GraphicsNode* node;
        QString       ownS;
        QString       otherS;
        QString       otherN;
    } Conn;

    QMap<int, Conn*> connections;

    QHash<QString, bool> toIgnore;

    auto loadConnections = [&connections](const QJsonArray& a, GraphicsNode* s) {
        for (int cId = 0; cId < a.size(); ++cId) {
            const auto c  = a[   cId       ].toObject();
            bool isSource = c[ QStringLiteral("direction") ] == QLatin1String("source");
            int  connId   = c[ QStringLiteral("id")        ].toInt() * (isSource ? 1 : -1);

            const auto nodeId = c[QStringLiteral("other_node")].toString();

            connections.insert(connId, new Conn {
                connId,                       /* id;      */
                isSource,                     /* isSource;*/
                s,                            /* node;    */
                c["own_socket"  ].toString(), /* ownS;    */
                c["other_socket"].toString(), /* otherS;  */
                nodeId,                       /* otherN;  */
            });

            Q_ASSERT(nodeId.size()==8 && nodeId == connections[connId]->otherN);
        }
    };

    for (int nodeId = 0; nodeId < nodes.size(); ++nodeId) {
        const QJsonObject nodeJ = nodes[nodeId].toObject();

        const auto widget = nodeJ [ QStringLiteral("widget") ].toObject();
        const auto data2  = nodeJ [ QStringLiteral("data")   ].toObject();
        const auto type   = data2 [ QStringLiteral("id")     ].toString();
        const auto uid    = widget[ QStringLiteral("UID")    ].toString();
        g_Used[uid] = true;

        if (uid.isEmpty()) {
            qWarning() << "Failed to properly load a node, some links may be missing";
            continue;
        }

        if (!m_hIdToType.contains(type)) {
            qWarning() << "Unknown node type" << type;
            toIgnore[uid] = true;
            continue;
        }

        if (auto metaInfo = m_hIdToType[type]) {

            Q_ASSERT(uid.size() == 8);

            auto pair = addToSceneFromMetaObject(metaInfo->m_spMetaObj, uid);
            pair.second->read(data2);

            auto nodeW = pair.first;

            nodeW->setRect(
                widget[ QStringLiteral("x")      ].toDouble(),
                widget[ QStringLiteral("y")      ].toDouble(),
                widget[ QStringLiteral("width")  ].toDouble(),
                widget[ QStringLiteral("height") ].toDouble()
            );

            nodeW->setDecoration(metaInfo->m_Icon);

            typedef void (GraphicsNode::*NodeSetter)(const QString&);

            auto get = [&widget](const QString& n) -> const QString
                {return widget[n].toString();};

            const QList<QPair<QString, NodeSetter>> mapper {
                {get("title"), &GraphicsNode::setTitle     },
                {get("bg"   ), &GraphicsNode::setBackground},
                {get("fg"   ), &GraphicsNode::setForeground}
            };

            for (const auto& prop : qAsConst(mapper) )
                if (!prop.first.isEmpty())
                    (nodeW->*prop.second)(prop.first);

            loadConnections(widget[QStringLiteral("connections")].toArray(), nodeW);

            toHash [nodeW] = uid;
            toNode [nodeW] = pair.second;

            if (auto m = const_cast<QAbstractItemModel*>(nodeW->index().model()))
                m->setData(nodeW->index(), uid, Qt::UserRole);
        }
    }

    auto edgeM = m_pNodeW->edgeModel();

    // Add the edges
    for (auto i = connections.begin(); i != connections.end(); ++i) {
        const auto conn = i.value();

        if (toIgnore.contains(conn->otherN))
            continue;

        if (conn->id >= 0) { // Sources
            // Validate symmetry
            auto otherconn = connections[-i.key()];

            if (otherconn && toIgnore.contains(otherconn->otherN))
                continue;

            Q_ASSERT(otherconn && otherconn != conn);
            Q_ASSERT(toHash[otherconn->node] == conn->otherN);
            Q_ASSERT(toHash[conn->node] == otherconn->otherN);

            const int row = edgeM->rowCount() -1;
            auto srcSock  = conn->node->sourceIndex(conn->ownS);
            auto sinkSock = otherconn->node->sinkIndex(otherconn->ownS);

            // Try to see if the node has a method to add
            if (auto an = toNode[conn->node])
                if ((!srcSock.isValid()) && an->createSourceSocket(conn->ownS))
                    srcSock  = conn->node->sourceIndex(conn->ownS);

            if (auto an = toNode[otherconn->node])
                if ((!sinkSock.isValid()) && an->createSinkSocket(otherconn->ownS))
                    sinkSock = otherconn->node->sinkIndex(otherconn->ownS);

            if ((!srcSock.isValid()) || (!sinkSock.isValid())) {
                qWarning() << "Failed to restore the connection between"
                    << conn->node->title() << ':' << conn->ownS << "and"
                    << otherconn->node->title() << ':' << otherconn->ownS;
                continue;
            }

            const bool ret1 = edgeM->setData(
                edgeM->index(row, 0),
                srcSock,
                QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
            );
            const bool ret2 = edgeM->setData(
                edgeM->index(row, 2),
                sinkSock,
                QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
            );

            // Validate if the connection is broken
            const auto lnode  = edgeM->index(row,0);

            const bool isValid = lnode.data(
                QReactiveProxyModel::ConnectionsRoles::IS_VALID
            ).toBool();

            // If both setData are a success, then the combination also is
            Q_ASSERT((ret1 && ret2) == isValid);

            if (!isValid) {
                qWarning() << "Failed to link the connection between"
                    << conn->node->title() << ':' << conn->ownS << "and"
                    << otherconn->node->title() << ':' << otherconn->ownS;
                continue;
            }

            delete conn;
            delete otherconn;
        }
    }

    const QJsonArray views = obj[QStringLiteral("views")].toArray();

    for (int i = 0; i < views.size(); ++i) {
        if (!d_ptr->m_lIS.isEmpty()) { //FIXME
            d_ptr->m_lIS[0]->read(views[i].toObject());
            d_ptr->m_lIS[0]->reflow();
        }
    }

}

void Session::load(QIODevice *dev)
{
    const QByteArray data = dev->readAll();
    load(data);
}


bool Session::addNodeFromData(const QByteArray& data, const QPoint& point)
{
    Q_UNUSED(point);

    QJsonDocument loadDoc(QJsonDocument::fromJson(data));
    const auto obj  = loadDoc.object();
    const auto type = obj[ QStringLiteral("id") ].toString();

    if (!m_hIdToType.contains(type)) {
        qWarning() << "Unknown node type" << type;
        return false;
    }

    // Give it a new UID
    obj[QStringLiteral("UID")] = QString();

    if (auto metaInfo = m_hIdToType[type]) {
        auto pair = addToSceneFromMetaObject(metaInfo->m_spMetaObj);

        pair.first->setRect({
            obj[QStringLiteral("x")     ].toDouble(),
            obj[QStringLiteral("y")     ].toDouble(),
            obj[QStringLiteral("width") ].toDouble(),
            obj[QStringLiteral("height")].toDouble()
        });

        pair.first->setTitle(obj[QStringLiteral("title") ].toString());
        pair.second->read(obj);

        m_pNodeW->setCurrentNode(pair.first);

        return true;
    }

    return false;
}

QByteArray Session::serializeSelection() const
{
    auto idx = m_pNodeW->selectionModel()->currentIndex();
    auto uid = idx.data(Qt::UserRole).toString();

    if ((!uid.isEmpty()) && m_hIdToNode.contains(uid)) {
        auto pair = m_hIdToNode[uid];

        QJsonObject o;

        const QRectF r = pair.first->rect();

        QJsonObject widget;
        pair.second->write(o);

        o[QStringLiteral("x")     ] = r.x();
        o[QStringLiteral("y")     ] = r.y();
        o[QStringLiteral("width") ] = r.width();
        o[QStringLiteral("height")] = r.height();
        o[QStringLiteral("title") ] = pair.first->title();
        o[QStringLiteral("UID")   ] = pair.first->index().data(Qt::UserRole).toString();

        const QByteArray a = QJsonDocument(o).toJson();

        return a;
    }

    return {};
}

AbstractNode* Session::fromGraphicsNode(GraphicsNode* n) const
{
    if (!n)
        return nullptr;

    Q_ASSERT(m_hIdToNode.contains(n->property("uid").toString()));

    return m_hIdToNode[n->property("uid").toString()].second;
}

GraphicsNode* Session::fromAbstractNode(AbstractNode* n) const
{
    if (!n)
        return nullptr;

    Q_ASSERT(m_hIdToNode.contains(n->property("uid").toString()));

    return m_hIdToNode[n->property("uid").toString()].first;
}

#include <session.moc>
