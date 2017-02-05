#include "session.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtCore/QTimer>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>

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

QPair<GraphicsNode*, AbstractNode*> Session::addToSceneFromMetaObject(const QMetaObject& meta, const QString& uid)
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

    QPair<GraphicsNode*, AbstractNode*> pair {n2, anode};

    const auto id = anode->id();

    m_hIdToType[id]->m_lInstances << pair;

    connect(anode, &QObject::destroyed, [anode, id, this]() {
        remove(anode, id);
    });

    return pair;
}

QPair<GraphicsNode*, AbstractNode*> Session::addToScene(const QModelIndex& idx)
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
            o[ "id"           ] = edgeI.data ().toInt   ();
            o[ "other_node"   ] = nodeid;

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
                node["data"] = data;

                const auto nodeW       = nodeJ.first;
                const auto sinkModel   = m_pNodeW->sinkSocketModel(nodeW->index());
                const auto sourceModel = m_pNodeW->sourceSocketModel(nodeW->index());

                const QRectF r = nodeW->rect();

                QJsonObject widget;
                widget["x"     ] = r.x();
                widget["y"     ] = r.y();
                widget["width" ] = r.width();
                widget["height"] = r.height();
                widget["title" ] = nodeW->title();
                widget["UID"   ] = nodeW->index().data(Qt::UserRole).toString();

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

    QJsonArray views;

    for (const auto is : qAsConst(d_ptr->m_lIS)) {
        QJsonObject view;
        is->write(view);
        views.append(view);
    }

    session["views"] = views;

    dev->write(QJsonDocument(session).toJson());
}

void Session::load(const QByteArray& data)
{
    QJsonDocument loadDoc(QJsonDocument::fromJson(data));

    const auto obj = loadDoc.object();

    const auto nodes = obj["nodes"].toArray();

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
            bool isSource = c[ "direction" ] == QLatin1String("source");
            int  connId   = c[ "id"        ].toInt() * (isSource ? 1 : -1);

            const auto nodeId = c["other_node"].toString();

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

        const auto widget = nodeJ [ "widget" ].toObject();
        const auto data2  = nodeJ [ "data"   ].toObject();
        const auto type   = data2 [ "id"     ].toString();
        const auto uid    = widget[ "UID"    ].toString();
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
                widget[ "x"      ].toDouble(),
                widget[ "y"      ].toDouble(),
                widget[ "width"  ].toDouble(),
                widget[ "height" ].toDouble()
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

            loadConnections(widget["connections"].toArray(), nodeW);

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
            auto srcSock  = conn->node->socketIndex(conn->ownS);
            auto sinkSock = otherconn->node->socketIndex(otherconn->ownS);

            // Try to see if the node has a method to add
            if (auto an = toNode[conn->node])
                if ((!srcSock.isValid()) && an->createSocket(conn->ownS)) {
                    conn->node->socketIndex(conn->ownS);
                    srcSock  = conn->node->socketIndex(conn->ownS);
                }

            if (auto an = toNode[otherconn->node])
                if ((!sinkSock.isValid()) && an->createSocket(otherconn->ownS)) {
                    otherconn->node->socketIndex(otherconn->ownS);
                    sinkSock = otherconn->node->socketIndex(otherconn->ownS);
                }

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

    const QJsonArray views = obj["views"].toArray();

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
