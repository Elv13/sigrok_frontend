#include "qnodeeditorsocketmodel.h"

#include <algorithm>

#include "graphicsnode.hpp"
#include "graphicsnodescene.hpp"
#include "graphicsbezieredge.hpp"
#include "graphicsbezieredge_p.h"

#include "qreactiveproxymodel.h"

#include "qmodeldatalistdecoder.h"

#include <QtCore/QDebug>
#include <QtCore/QMimeData>
#include <QtCore/QSortFilterProxyModel>

#include "qobjectmodel.h" //TODO remove

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class QNodeEdgeFilterProxy;

struct EdgeWrapper;
struct SocketWrapper;

struct NodeWrapper final
{
    NodeWrapper(QNodeEditorSocketModel *q, const QPersistentModelIndex& i) : m_Node(q, i) {}

    GraphicsNode m_Node;

    // Keep aligned with the source row
    QVector<int> m_lSourcesFromSrc {};
    QVector<int> m_lSinksFromSrc {};

    QVector<int> m_lSourcesToSrc {};
    QVector<int> m_lSinksToSrc {};

    // Keep aligned with the node row
    QVector<SocketWrapper*> m_lSources {};
    QVector<SocketWrapper*> m_lSinks {};

    mutable QNodeEdgeFilterProxy *m_pSourceProxy {Q_NULLPTR};
    mutable QNodeEdgeFilterProxy *m_pSinkProxy {Q_NULLPTR};

    QRectF m_SceneRect;
};

struct SocketWrapper final
{
    SocketWrapper(const QModelIndex& idx, GraphicsNodeSocket::SocketType t, NodeWrapper* n)
        : m_Socket(idx, t, &n->m_Node), m_pNode(n) {}

    GraphicsNodeSocket m_Socket;

    NodeWrapper* m_pNode;

    EdgeWrapper* m_EdgeWrapper {Q_NULLPTR};
};

//TODO split the "data" and "proxy" part of this class and use it to replace
// the NodeWrapper:: content. This way, getting an on demand proxy will cost
// "nothing"
class QNodeEdgeFilterProxy final : public QAbstractProxyModel
{
    Q_OBJECT
public:
    explicit QNodeEdgeFilterProxy(QNodeEditorSocketModelPrivate* d, NodeWrapper *w, GraphicsNodeSocket::SocketType t);

    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

//     void updateExtraColumns(

private:
    const GraphicsNodeSocket::SocketType m_Type;
    const NodeWrapper *m_pWrapper;

    // Helpers
    void processRow(const QModelIndex& srcIdx);

    QNodeEditorSocketModelPrivate* d_ptr;
};

struct EdgeWrapper final
{
    explicit EdgeWrapper(QNodeEditorEdgeModel* m, const QModelIndex& index)
        : m_Edge(m, index)
    { Q_ASSERT(index.isValid()); }

    SocketWrapper*     m_pSource {Q_NULLPTR};
    GraphicsBezierEdge m_Edge               ;
    SocketWrapper*     m_pSink   {Q_NULLPTR};
    bool               m_IsShown {  false  };
    int                m_EdgeId  {s_CurId++};

    static int s_CurId;
};

class QNodeEditorSocketModelPrivate final : public QObject
{
    Q_OBJECT
public:
    enum class State {
        NORMAL,
        DRAGGING,
    };

    explicit QNodeEditorSocketModelPrivate(QObject* p) : QObject(p) {}

    QNodeEditorEdgeModel  m_EdgeModel {this};
    QVector<NodeWrapper*> m_lWrappers;
    QVector<EdgeWrapper*> m_lEdges;
    GraphicsNodeScene*    m_pScene;
    State                 m_State {State::NORMAL};
    quint32               m_CurrentTypeId {QMetaType::UnknownType};
    bool                  m_IsSingleEdge {true};

    // helper
    GraphicsNode* insertNode(int idx);
    NodeWrapper*  getNode(const QModelIndex& idx, bool r = false) const;

    void insertSockets(const QModelIndex& parent, int first, int last);
    void updateSockets(const QModelIndex& parent, int first, int last);

    GraphicsDirectedEdge* initiateConnectionFromSource(
        const QModelIndex&             index,
        GraphicsNodeSocket::SocketType type,
        const QPointF&                 point
    );

    // Use a template so the compiler can safely inline the result
    template<
        QVector<int> NodeWrapper::* SM,
        QVector<SocketWrapper*> NodeWrapper::* S,
        SocketWrapper* EdgeWrapper::* E
    >
    inline SocketWrapper* getSocketCommon(const QModelIndex& idx) const;

    SocketWrapper* getSourceSocket(const QModelIndex& idx) const;
    SocketWrapper* getSinkSocket(const QModelIndex& idx) const;

    QNodeEditorSocketModel* q_ptr;

public Q_SLOTS:
    void slotRowsInserted       (const QModelIndex& parent, int first, int last);
    void slotConnectionsInserted(const QModelIndex& parent, int first, int last);
    void slotConnectionsChanged (const QModelIndex& tl, const QModelIndex& br  );
    void slotAboutRemoveItem    (const QModelIndex &parent, int first, int last);
    void slotConnectionsAboutToReset();
    void exitDraggingMode();
};

int EdgeWrapper::s_CurId = 1;

QNodeEditorSocketModel::QNodeEditorSocketModel( QReactiveProxyModel* rmodel, GraphicsNodeScene* scene ) : 
    QTypeColoriserProxy(rmodel), d_ptr(new QNodeEditorSocketModelPrivate(this))
{
    Q_ASSERT(rmodel);

    setBackgroundRole<bool>(QBrush("#ff0000"));
    setBackgroundRole<int>(QBrush("#ff00ff"));
    setBackgroundRole<QAbstractItemModel*>(QBrush("#ffff00"));

    rmodel->addConnectedRole(QObjectModel::Role::ValueRole);

    d_ptr->q_ptr    = this;
    d_ptr->m_pScene = scene;
    setSourceModel(rmodel);

    d_ptr->m_EdgeModel.setSourceModel(rmodel->connectionsModel());

    connect(this, &QAbstractItemModel::rowsInserted,
        d_ptr, &QNodeEditorSocketModelPrivate::slotRowsInserted);

    connect(this, &QAbstractItemModel::rowsAboutToBeRemoved,
        d_ptr, &QNodeEditorSocketModelPrivate::slotAboutRemoveItem);

    connect(&d_ptr->m_EdgeModel, &QAbstractItemModel::rowsInserted,
        d_ptr, &QNodeEditorSocketModelPrivate::slotConnectionsInserted);

    connect(&d_ptr->m_EdgeModel, &QAbstractItemModel::modelAboutToBeReset,
        d_ptr, &QNodeEditorSocketModelPrivate::slotConnectionsAboutToReset);

    connect(&d_ptr->m_EdgeModel, &QAbstractItemModel::dataChanged,
        d_ptr, &QNodeEditorSocketModelPrivate::slotConnectionsChanged);

    rmodel->setCurrentProxy(this);
}

QNodeEditorSocketModel::~QNodeEditorSocketModel()
{
    // Delete all edges
    d_ptr->slotConnectionsAboutToReset();

    while (!d_ptr->m_lWrappers.isEmpty()) {
        auto node = d_ptr->m_lWrappers.takeLast();

        for (auto sw : qAsConst(node->m_lSources))
            delete sw;

        for (auto sw : qAsConst(node->m_lSinks))
            delete sw;

        node->m_lSources.clear();
        node->m_lSinks.clear();

        //d_ptr->m_pScene->removeItem(node->m_Node.graphicsItem());

        delete node;
    }

    delete d_ptr;
}

void QNodeEditorSocketModel::setSourceModel(QAbstractItemModel *sm)
{
    // This models can only work with a QReactiveProxyModel (no proxies)
    Q_ASSERT(qobject_cast<QReactiveProxyModel*>(sm));

    QTypeColoriserProxy::setSourceModel(sm);

    //TODO clear (this can wait, it wont happen anyway)

    d_ptr->slotRowsInserted({}, 0, sourceModel()->rowCount() -1 );
}

bool QNodeEditorSocketModel::setData(const QModelIndex &idx, const QVariant &value, int role)
{
    if (!idx.isValid())
        return false;

    if (role == Qt::SizeHintRole && value.canConvert<QRectF>() && !idx.parent().isValid()) {
        auto n = d_ptr->getNode(idx);
        Q_ASSERT(n);
        n->m_SceneRect = value.toRectF();

        Q_EMIT dataChanged(idx, idx);

        // All socket position also changed
        const int cc = rowCount(idx);

        if (cc)
            Q_EMIT dataChanged(index(0,0,idx), index(cc -1, 0, idx));

        return true;
    }

    return QTypeColoriserProxy::setData(idx, value, role);
}

QMimeData *QNodeEditorSocketModel::mimeData(const QModelIndexList &idxs) const
{
    auto md = QTypeColoriserProxy::mimeData(idxs);

    // Assume the QMimeData exist only while the data is being dragged
    if (md) {
        const QModelDataListDecoder decoder(md);

        auto typeId = decoder.typeId(Qt::EditRole);

        if (typeId != QMetaType::UnknownType) {
            d_ptr->m_State = QNodeEditorSocketModelPrivate::State::DRAGGING;
            d_ptr->m_CurrentTypeId = typeId;

            connect(md, &QObject::destroyed, d_ptr,
                &QNodeEditorSocketModelPrivate::exitDraggingMode);

            for (auto n : qAsConst(d_ptr->m_lWrappers))
                n->m_Node.update();
        }
    }

    return md;
}

Qt::ItemFlags QNodeEditorSocketModel::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags f = QTypeColoriserProxy::flags(idx);

    const auto editRole = idx.data(Qt::EditRole);

    // Disable everything but compatible sockets. Also allow sockets with
    // unknown types as they are probably generic sockets where the type
    // is irrelevant.
    return f ^ ((
        d_ptr->m_State == QNodeEditorSocketModelPrivate::State::DRAGGING &&
        (
            editRole.userType() != QMetaType::QVariant &&
            editRole.userType() != QMetaType::UnknownType &&
            (!editRole.canConvert(d_ptr->m_CurrentTypeId))
        ) &&
        f | Qt::ItemIsEnabled
    ) ? Qt::ItemIsEnabled : Qt::NoItemFlags);
}

void QNodeEditorSocketModelPrivate::exitDraggingMode()
{
    m_CurrentTypeId = QMetaType::UnknownType;
    m_State = QNodeEditorSocketModelPrivate::State::NORMAL;

    for (auto n : qAsConst(m_lWrappers))
        n->m_Node.update();
}

GraphicsNodeScene* QNodeEditorSocketModel::scene() const
{
    return d_ptr->m_pScene;
}

GraphicsNodeScene* QNodeEditorEdgeModel::scene() const
{
    return d_ptr->m_pScene;
}

int QNodeEditorSocketModel::sourceSocketCount(const QModelIndex& idx) const
{
    const auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSources.size() : 0;
}

int QNodeEditorSocketModel::sinkSocketCount(const QModelIndex& idx) const
{
    const auto nodew = d_ptr->getNode(idx);

    return nodew ? nodew->m_lSinks.size() : 0;
}

GraphicsNode* QNodeEditorSocketModel::getNode(const QModelIndex& idx, bool recursive)
{
    if ((!idx.isValid()) || idx.model() != this)
        return Q_NULLPTR;

    const auto i = (recursive && idx.parent().isValid()) ? idx.parent() : idx;

    auto nodew = d_ptr->getNode(i);

    return nodew ? &nodew->m_Node : Q_NULLPTR;
}

template<
    QVector<int> NodeWrapper::* SM,
    QVector<SocketWrapper*> NodeWrapper::* S,
    SocketWrapper* EdgeWrapper::* E
>
SocketWrapper* QNodeEditorSocketModelPrivate::getSocketCommon(const QModelIndex& idx) const
{
    // Use some dark template metamagic. This could have been done otherwise
    if (!idx.parent().isValid())
        return Q_NULLPTR;

    if (idx.model() == q_ptr->edgeModel()) {
        const EdgeWrapper* e = m_lEdges[idx.row()];
        return (*e).*E;
    }

    const NodeWrapper* nodew = getNode(idx, true);

    if (!nodew)
        return Q_NULLPTR;

    const int count = ((*nodew).*SM).size();

    // The -1 is because int defaults to 0 and invalid is -1
    const int relIdx = count > idx.row() ?
        ((*nodew).*SM)[idx.row()] - 1 : -1;

    // This should never happen, but is "recoverable"
    if (relIdx >= count) {
        qWarning() << "Trying to access a socket that should exist, but doesn't";
        return Q_NULLPTR;
    }

    auto ret = relIdx != -1 ? ((*nodew).*S)[relIdx] : Q_NULLPTR;

//     Q_ASSERT((!ret) || ret->m_Socket.index() == idx);

    return ret;
}

SocketWrapper* QNodeEditorSocketModelPrivate::getSourceSocket(const QModelIndex& idx) const
{
    return getSocketCommon<
        &NodeWrapper::m_lSourcesFromSrc, &NodeWrapper::m_lSources, &EdgeWrapper::m_pSource
    >(idx);
}

SocketWrapper* QNodeEditorSocketModelPrivate::getSinkSocket(const QModelIndex& idx) const
{
    return getSocketCommon<
        &NodeWrapper::m_lSinksFromSrc, &NodeWrapper::m_lSinks, &EdgeWrapper::m_pSink
    >(idx);
}

GraphicsNodeSocket* QNodeEditorSocketModel::getSourceSocket(const QModelIndex& idx)
{
    return &d_ptr->getSocketCommon<
        &NodeWrapper::m_lSourcesFromSrc, &NodeWrapper::m_lSources, &EdgeWrapper::m_pSource
    >(idx)->m_Socket;
}

GraphicsNodeSocket* QNodeEditorSocketModel::getSinkSocket(const QModelIndex& idx)
{
    return &d_ptr->getSocketCommon<
        &NodeWrapper::m_lSinksFromSrc, &NodeWrapper::m_lSinks, &EdgeWrapper::m_pSink
    >(idx)->m_Socket;
}

GraphicsDirectedEdge* QNodeEditorSocketModel::getSourceEdge(const QModelIndex& idx)
{
    return idx.model() == edgeModel() ?
        &d_ptr->m_lEdges[idx.row()]->m_Edge : Q_NULLPTR;

    //FIXME support SocketModel index
}

GraphicsDirectedEdge* QNodeEditorSocketModel::getSinkEdge(const QModelIndex& idx)
{
    return idx.model() == edgeModel() ?
        &d_ptr->m_lEdges[idx.row()]->m_Edge : Q_NULLPTR;

    //FIXME support SocketModel index
}

QNodeEditorEdgeModel* QNodeEditorSocketModel::edgeModel() const
{
    return &d_ptr->m_EdgeModel;
}


bool QNodeEditorSocketModel::isSingleEdgeEnforced() const
{
    return d_ptr->m_IsSingleEdge;
}

void QNodeEditorSocketModel::setEnforceSingleEdge(bool value)
{
    d_ptr->m_IsSingleEdge = value;
}

GraphicsDirectedEdge* QNodeEditorSocketModelPrivate::initiateConnectionFromSource( const QModelIndex& idx, GraphicsNodeSocket::SocketType type, const QPointF& point )
{
    Q_UNUSED(type ); //TODO use it or delete it
    Q_UNUSED(point); //TODO use it or delete it

    if (!idx.parent().isValid()) {
        qWarning() << "Cannot initiate an edge from an invalid node index";
        return Q_NULLPTR;
    }

    const int last = q_ptr->edgeModel()->rowCount() - 1;

    if (m_lEdges.size() <= last || !m_lEdges[last]) {
        m_lEdges.resize(std::max(m_lEdges.size(), last+1));
        m_lEdges[last] = new EdgeWrapper(
            q_ptr->edgeModel(),
            q_ptr->edgeModel()->index(last, 1)
        );
        m_pScene->addItem(m_lEdges[last]->m_Edge.graphicsItem());
    }

    return &m_lEdges[last]->m_Edge;
}

GraphicsDirectedEdge* QNodeEditorSocketModel::initiateConnectionFromSource(const QModelIndex& index, const QPointF& point)
{
    return d_ptr->initiateConnectionFromSource(
        index, GraphicsNodeSocket::SocketType::SOURCE, point
    );
}

GraphicsDirectedEdge* QNodeEditorSocketModel::initiateConnectionFromSink(const QModelIndex& index, const QPointF& point)
{
    return d_ptr->initiateConnectionFromSource(
        index, GraphicsNodeSocket::SocketType::SINK, point
    );
}

QAbstractItemModel *QNodeEditorSocketModel::sinkSocketModel(const QModelIndex& node) const
{
    auto n = d_ptr->getNode(node);

    if (!n)
        return Q_NULLPTR;

    if (!n->m_pSinkProxy)
        n->m_pSinkProxy = new QNodeEdgeFilterProxy(
            d_ptr,
            n,
            GraphicsNodeSocket::SocketType::SINK
        );

    return n->m_pSinkProxy;
}

QAbstractItemModel *QNodeEditorSocketModel::sourceSocketModel(const QModelIndex& node) const
{
    auto n = d_ptr->getNode(node);

    if (!n)
        return Q_NULLPTR;

    if (!n->m_pSourceProxy)
        n->m_pSourceProxy = new QNodeEdgeFilterProxy(
            d_ptr,
            n,
            GraphicsNodeSocket::SocketType::SOURCE
        );

    return n->m_pSourceProxy;
}

void QNodeEditorSocketModelPrivate::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    if (last < first) return;

    if (!parent.isValid()) {
        // create new nodes
        for (int i = first; i <= last; i++) {
            const auto idx = q_ptr->index(i, 0);
            if (idx.isValid()) {
                insertNode(idx.row());
                slotRowsInserted(idx, 0, q_ptr->rowCount(idx) - 1);
            }
        }
    }
    else if (!parent.parent().isValid()) {
        if (auto nodew = getNode(parent)) {
            const int delta = (last - first) + 1;

            // Sync the mapping from the socket row to the source row
            for (int i = first; i < nodew->m_lSourcesToSrc.size(); i++)
                nodew->m_lSourcesToSrc[i] += delta;
            for (int i = first; i < nodew->m_lSinksToSrc.size(); i++)
                nodew->m_lSinksToSrc[i]   += delta;

            // Sync the source row -> socket row table
            for (int i = first; i <= last; i++) {
                nodew->m_lSourcesFromSrc.insert(first, 0);
                nodew->m_lSinksFromSrc.insert(first, 0);
            }

            insertSockets(parent, first, last);
        }
    }
}

GraphicsNode* QNodeEditorSocketModelPrivate::insertNode(int idx)
{
    const auto idx2 = q_ptr->index(idx, 0);

    Q_ASSERT(idx2.isValid());

    if (idx == 0 && m_lWrappers.size())
        Q_ASSERT(false);

    auto nw = new NodeWrapper(q_ptr, idx2);

    m_lWrappers.insert(idx, nw);

    m_pScene->addItem(nw->m_Node.graphicsItem());

    return &nw->m_Node;
}

NodeWrapper* QNodeEditorSocketModelPrivate::getNode(const QModelIndex& idx, bool r) const
{
    // for convenience
    auto i = idx.model() == q_ptr->sourceModel() ? q_ptr->mapFromSource(idx) : idx;

    if ((!i.isValid()) || i.model() != q_ptr)
        return Q_NULLPTR;

    if (i.parent().isValid() && r)
        i = i.parent();

    if (i.parent().isValid())
        return Q_NULLPTR;

    Q_ASSERT(i == q_ptr->index(i.row(), i.column()));
    Q_ASSERT(i.row() < q_ptr->rowCount());

    // This should have been taken care of already. If it isn't, either the
    // source model is buggy (it will cause crashes later anyway) or this
    // code is (and in that case, the state is already corrupted, ignoring that
    // will cause garbage data to be shown/serialized).
    Q_ASSERT(m_lWrappers.size() > i.row());

    return m_lWrappers[i.row()];
}

void QNodeEditorSocketModelPrivate::insertSockets(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(first)
    Q_UNUSED(last)

    auto nodew = getNode(parent);
    Q_ASSERT(nodew);

    Q_ASSERT(parent.isValid() && (!parent.parent().isValid()));

    Q_ASSERT(parent.model() == q_ptr);

    Q_ASSERT(nodew->m_lSinksFromSrc.size() >  last);
    Q_ASSERT(nodew->m_lSourcesFromSrc.size() >  last);

    for (int i = first; i <= last; i++) {
        const auto idx = q_ptr->index(i, 0, parent);

        // It doesn't attempt to insert the socket at the correct index as
        // many items will be rejected

        constexpr static const Qt::ItemFlags sourceFlags(
            Qt::ItemIsDragEnabled |
            Qt::ItemIsSelectable
        );

        // SOURCES
        if ((idx.flags() & sourceFlags) == sourceFlags) {
            auto s = new SocketWrapper(
                idx,
                GraphicsNodeSocket::SocketType::SOURCE,
                nodew
            );


            nodew->m_lSourcesFromSrc[i] = nodew->m_lSources.size() + 1;
            nodew->m_lSources << s;
            nodew->m_lSourcesToSrc << i;
        }

        constexpr static const Qt::ItemFlags sinkFlags(
            Qt::ItemIsDropEnabled |
            Qt::ItemIsSelectable  |
            Qt::ItemIsEditable
        );

        // SINKS
        if ((idx.flags() & sinkFlags) == sinkFlags) {
            auto s = new SocketWrapper(
                idx,
                GraphicsNodeSocket::SocketType::SINK,
                nodew
            );

            nodew->m_lSinksFromSrc[i] = nodew->m_lSinks.size() + 1;
            nodew->m_lSinks << s;
            nodew->m_lSinksToSrc << i;
        }

    }

    nodew->m_Node.update();

    Q_ASSERT(nodew->m_lSinksFromSrc.size() == nodew->m_lSourcesFromSrc.size());
}

void QNodeEditorSocketModelPrivate::updateSockets(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)
    Q_UNUSED(first)
    Q_UNUSED(last)
    //TODO
}

QNodeEditorEdgeModel::QNodeEditorEdgeModel(QNodeEditorSocketModelPrivate* parent)
    : QIdentityProxyModel(parent), d_ptr(parent)
{

}

QNodeEditorEdgeModel::~QNodeEditorEdgeModel()
{ /* Nothing is owned by the edge model*/ }

bool QNodeEditorEdgeModel::canConnect(const QModelIndex& idx1, const QModelIndex& idx2) const
{
    Q_UNUSED(idx1)
    Q_UNUSED(idx2)

    return true; //TODO
}

bool QNodeEditorEdgeModel::connectSocket(const QModelIndex& idx1, const QModelIndex& idx2)
{
    if (idx1.model() != d_ptr->q_ptr || idx2.model() != d_ptr->q_ptr)
        return false;

    auto m = qobject_cast<QReactiveProxyModel*>(d_ptr->q_ptr->sourceModel());

    m->connectIndices(
        d_ptr->q_ptr->mapToSource(idx1),
        d_ptr->q_ptr->mapToSource(idx2)
    );

    return true;
}

bool QNodeEditorEdgeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    // Prevent sink-to-sink and source-to-source connections. Otherwise the
    // SocketModel can be invalid while the reactive one is
    auto i = value.toModelIndex();

    // Exit early to keep the code simple. It wont work anyway unless someone
    // use this in very "creative" ways.
    if (i.isValid() && !i.parent().isValid()) {
        qWarning() << "Trying to use a node as a socket"
            << index.data() << i.data();
        return false;
    }

    // Prevent already connected sockets to be connected again. In many
    // scenarios, having multiple edges bring more complexity than sense. So
    // it is better to prevent it altogether.
    if (d_ptr->m_IsSingleEdge) {
        Q_ASSERT((!i.isValid()) || i.model() == d_ptr->q_ptr);
        switch(index.column()) {
            case 0: {
                const auto self  = d_ptr->getSinkSocket(index);
                const auto other = d_ptr->getSourceSocket(i);

                // If `i` isn't valid, then it's a disconnection
                if (Q_UNLIKELY(i.isValid() && self)) {
                    qWarning() << "The sink socket is already connected"
                        << index.data() << i.data();
                    return false;
                }

                if (Q_UNLIKELY(i.isValid() && !other)) {
                    qWarning() << "The other socket doesn't exist"
                        << index.data() << i.data();
                    return false;
                }

                // Other can be nil of the edge is dropped on empty space
                if (Q_UNLIKELY(other && other->m_EdgeWrapper)) {
                    qWarning() << "The source socket is already connected"
                        << index.data() << i.data();
                    return false;
                }

            } break;
            case 2: {
                const auto self  = d_ptr->getSourceSocket(index);
                const auto other = d_ptr->getSinkSocket(i);

                // If `i` isn't valid, then it's a disconnection
                if (Q_UNLIKELY(i.isValid() && self)) {
                    qWarning() << "The source socket is already connected"
                        << index.data() << i.data();
                    return false;
                }

                if (Q_UNLIKELY(i.isValid() && !other)) {
                    qWarning() << "The other socket doesn't exist"
                        << index.data() << i.data();
                    return false;
                }

                // Other can be nil of the edge is dropped on empty space
                if (Q_UNLIKELY(other && other->m_EdgeWrapper)) {
                    qWarning() << "The sink socket is already connected"
                        << index.data() << i.data();
                    return false;
                }

            } break;
        };
    }

    if (i.isValid()) {
        switch(index.column()) {
            case 0:
                if (!((i.flags() & Qt::ItemIsDragEnabled))) {
                    qWarning() << "Trying to use a source as a sink"
                        << index.data() << i.data();

                    return false;
                }
                break;
            case 2:
                if (!((i.flags() & Qt::ItemIsDropEnabled))) {
                    qWarning() << "Trying to use a sink as a source"
                        << index.data() << i.data();

                    return false;
                }
                break;
        }
    }

    return QIdentityProxyModel::setData(index, value, role);
}

QVariant QNodeEditorEdgeModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    Q_ASSERT(idx.model() == this);

    QModelIndex srcIdx;

    switch(idx.column()) {
        case QReactiveProxyModel::ConnectionsColumns::SOURCE:
            srcIdx = mapToSource(idx).data(
                QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
            ).toModelIndex();
            break;
        case QReactiveProxyModel::ConnectionsColumns::DESTINATION:
            srcIdx = mapToSource(idx).data(
                QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
            ).toModelIndex();
            break;
        case QReactiveProxyModel::ConnectionsColumns::CONNECTION:

            srcIdx = mapToSource(index(idx.row(),2)).data(
                QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
            ).toModelIndex();

            // Use the socket background as line color
            if (role != Qt::ForegroundRole)
                break;

            if (!srcIdx.isValid()) {
                srcIdx = mapToSource(index(idx.row(),0)).data(
                    QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
                ).toModelIndex();
            }

            return d_ptr->q_ptr->mapFromSource(srcIdx).data(Qt::BackgroundRole);
    }

    if (srcIdx.model() == d_ptr->q_ptr->sourceModel())
        srcIdx = d_ptr->q_ptr->mapFromSource(srcIdx);

    // It will crash soon if it is false, better do it now
    Q_ASSERT(
        (!srcIdx.isValid()) ||
        srcIdx.model() == d_ptr->q_ptr ||
        srcIdx.model() == d_ptr->q_ptr->edgeModel()
    );

    if (srcIdx.model() == d_ptr->q_ptr && srcIdx.parent().isValid()) //TODO remove
        Q_ASSERT(srcIdx.parent().data() == d_ptr->q_ptr->index(srcIdx.parent().row(),0).data());

    auto sock = (!idx.column()) ?
        d_ptr->getSourceSocket(srcIdx) : d_ptr->getSinkSocket(srcIdx);

    if (sock && role == Qt::SizeHintRole)
        return sock->m_Socket.graphicsItem()->mapToScene(0,0);

    return QIdentityProxyModel::data(idx, role);
}

void QNodeEditorSocketModelPrivate::slotConnectionsInserted(const QModelIndex& parent, int first, int last)
{
    //FIXME dead code
    typedef QReactiveProxyModel::ConnectionsColumns Column;

    auto srcSockI  = q_ptr->index(first, Column::SOURCE).data(
        QReactiveProxyModel::ConnectionsRoles::SOURCE_INDEX
    );
    auto destSockI = q_ptr->index(first, Column::DESTINATION).data(
        QReactiveProxyModel::ConnectionsRoles::DESTINATION_INDEX
    );

    // If this happens, then the model is buggy or there is a race condition.
    if ((!srcSockI.isValid()) && (!destSockI.isValid()))
        return;

    //TODO support rows removed

    // Avoid pointless indentation
    if (last > first)
        slotConnectionsInserted(parent, ++first, last);
}

void QNodeEditorSocketModelPrivate::slotConnectionsAboutToReset()
{
    // Clear all edges references
    for (auto nw : qAsConst(m_lWrappers)) {
        for (auto sw : qAsConst(nw->m_lSources))
            sw->m_EdgeWrapper = Q_NULLPTR;
        for (auto sw : qAsConst(nw->m_lSinks))
            sw->m_EdgeWrapper = Q_NULLPTR;
    }

    for (auto ew : qAsConst(m_lEdges))
        delete ew;

    m_lEdges.clear();
}

void QNodeEditorSocketModelPrivate::slotConnectionsChanged(const QModelIndex& tl, const QModelIndex& br)
{
    typedef QReactiveProxyModel::ConnectionsRoles CRole;

    for (int i = tl.row(); i <= br.row(); i++) {
        const auto src  = m_EdgeModel.index(i, 0).data(CRole::SOURCE_INDEX     ).toModelIndex();
        const auto sink = m_EdgeModel.index(i, 2).data(CRole::DESTINATION_INDEX).toModelIndex();

        // Make sure the edge exists
        if (m_lEdges.size()-1 < i || !m_lEdges[i]) {
            m_lEdges.resize(std::max(m_lEdges.size(), i+1));
            m_lEdges[i] = new EdgeWrapper(&m_EdgeModel, m_EdgeModel.index(i, 1));
        }

        auto e = m_lEdges[i];

        auto oldSrc(e->m_pSource), oldSink(e->m_pSink);

        // Update the node mapping
        if ((e->m_pSource = getSourceSocket(src)))
            e->m_pSource->m_Socket.setEdge(m_EdgeModel.index(i, 0));

        if (oldSrc != e->m_pSource) {
            if (oldSrc)
                oldSrc->m_Socket.setEdge({});

            if (oldSrc && oldSrc->m_EdgeWrapper && oldSrc->m_EdgeWrapper == e) {
                oldSrc->m_EdgeWrapper = Q_NULLPTR;
            }

            if (e->m_pSource) {

                // Assert early to prevent the model from going into an invalid
                // state. `setData` should have prevented this already.
                if (m_IsSingleEdge)
                    Q_ASSERT((!e->m_pSource->m_EdgeWrapper)
                        || (e->m_pSource->m_EdgeWrapper == e));

                e->m_pSource->m_EdgeWrapper = e;
            }
        }
        if ((e->m_pSink = getSinkSocket(sink)))
            e->m_pSink->m_Socket.setEdge(m_EdgeModel.index(i, 2));

        Q_ASSERT((!sink.isValid()) || (sink.isValid() && e->m_pSink));
        Q_ASSERT((!src.isValid()) || (src.isValid() && e->m_pSource));

        if (oldSink != e->m_pSink) {
            if (oldSink)
                oldSink->m_Socket.setEdge({});

            if (oldSink && oldSink->m_EdgeWrapper && oldSink->m_EdgeWrapper == e) {
                oldSink->m_EdgeWrapper = Q_NULLPTR;
            }

            if (e->m_pSink) {

                // Assert early to prevent the model from going into an invalid
                // state. `setData` should have prevented this already.
                if (m_IsSingleEdge)
                    Q_ASSERT((!e->m_pSink->m_EdgeWrapper)
                        || (e->m_pSink->m_EdgeWrapper == e));

                e->m_pSink->m_EdgeWrapper = e;
            }
        }

        // Update the graphic item
        const bool isUsed = e->m_pSource || e->m_pSink;

        e->m_Edge.update();

        if (e->m_IsShown != isUsed && isUsed)
            m_pScene->addItem(e->m_Edge.graphicsItem());
        else if (e->m_IsShown != isUsed && !isUsed)
            m_pScene->removeItem(e->m_Edge.graphicsItem());

        e->m_IsShown = isUsed;
    }
}

void QNodeEditorSocketModelPrivate::slotAboutRemoveItem(const QModelIndex &parent, int first, int last)
{
    if (first < 0 || last < first)
        return;

    if (!parent.isValid() && m_lWrappers.size() > last) {

        for (int i = first; i <= last; i++) {
            const auto idx = q_ptr->index(i, 0, parent);

            // remove the sockets
            slotAboutRemoveItem(idx, 0, q_ptr->rowCount(idx) - 1);

            auto nw = m_lWrappers[i];
            nw->m_Node.setCentralWidget(Q_NULLPTR);
            m_pScene->removeItem(nw->m_Node.graphicsItem());

            m_lWrappers.remove(i - (i-first));

            delete nw;
        }
    }
    else if (parent.isValid() && !parent.parent().isValid()) {
        auto nw = m_lWrappers[parent.row()];
        Q_ASSERT(nw);

        QList<int> srcToDel, sinkToDel;

        for (int i = first; i <= last; i++) {
            Q_ASSERT(parent == nw->m_Node.index());

            Q_ASSERT(nw->m_lSinksFromSrc.size() == nw->m_lSourcesFromSrc.size());
            Q_ASSERT(nw->m_lSourcesFromSrc.size() > q_ptr->rowCount(parent) - 1);

            int sid = nw->m_lSourcesFromSrc[i] - 1;
            if (sid >= 0) {
                auto sw = nw->m_lSources[sid];
                m_pScene->removeItem(sw->m_Socket.graphicsItem());
                srcToDel << sid;
                delete sw;
            }

            sid = nw->m_lSinksFromSrc[i] - 1;

            if (sid >= 0) {
                auto sw = nw->m_lSinks[sid];
                m_pScene->removeItem(sw->m_Socket.graphicsItem());
                sinkToDel << sid;
                delete sw;
            }
        }

        // The code below subtract `i` to compensate for the shrinking list.
        // chances are items will be ordered, but there is case where it might
        // not be the case
        std::sort(srcToDel.begin() , srcToDel.end() );
        std::sort(sinkToDel.begin(), sinkToDel.end());

        nw->m_lSinksFromSrc.remove(first, last - first + 1);
        nw->m_lSourcesFromSrc.remove(first, last - first + 1);

        // remove from the list, this assume the sockets are ordered
        for (int i = 0; i < srcToDel.size(); i++) {
            nw->m_lSources.remove(srcToDel[i] - i);
            nw->m_lSourcesToSrc.remove(srcToDel[i] - i);
        }

        for (int i = 0; i < sinkToDel.size(); i++) {
            nw->m_lSinks.remove(sinkToDel[i] - i);
            nw->m_lSinksToSrc.remove(sinkToDel[i] - i);
        }

        // reload the "FromSrc" mapping
        int rc(q_ptr->rowCount(parent) - (last - first) - 1), csrc(0), csink(0);
        for (int i = 0; i < rc; i++) {
            const auto idx2 = q_ptr->index(i, 0, parent);

            nw->m_lSourcesFromSrc[i] = (csrc < nw->m_lSources.size()
                && nw->m_lSources[csrc]->m_Socket.index() == idx2
                && ++csrc) ? csrc : -1;

            nw->m_lSinksFromSrc[i] = (csink < nw->m_lSinks.size()
                && nw->m_lSinks[csink]->m_Socket.index() == idx2
                && ++csink) ? csink : -1;
        }
    }
}

QNodeEditorSocketModel* QNodeEditorEdgeModel::socketModel() const
{
    return d_ptr->q_ptr;
}

QNodeEdgeFilterProxy::QNodeEdgeFilterProxy(QNodeEditorSocketModelPrivate* d, NodeWrapper *w, GraphicsNodeSocket::SocketType t) :
    QAbstractProxyModel(d), m_Type(t), m_pWrapper(w), d_ptr(d)
{
    setSourceModel(d->q_ptr);
}

int QNodeEdgeFilterProxy::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
        return 0;

    switch (m_Type) {
        case GraphicsNodeSocket::SocketType::SOURCE:
            return m_pWrapper->m_lSources.size();
        case GraphicsNodeSocket::SocketType::SINK:
            return m_pWrapper->m_lSinks.size();
    }

    return 0;
}

QModelIndex QNodeEdgeFilterProxy::mapFromSource(const QModelIndex& srcIdx) const
{
    static const auto check = [](
        const QVector<int>& l, const QModelIndex& src, const GraphicsNode* n
    ) -> bool {
        return src.isValid()
            && l.size() > src.row()
            && n->index() == src.parent()
            && l[src.row()];
    };

    switch (m_Type) {
        case GraphicsNodeSocket::SocketType::SOURCE:
            if (check(m_pWrapper->m_lSourcesFromSrc, srcIdx, &m_pWrapper->m_Node))
                return createIndex(m_pWrapper->m_lSourcesFromSrc[srcIdx.row()] - 1, 0, Q_NULLPTR);
            break;
        case GraphicsNodeSocket::SocketType::SINK:
            if (check(m_pWrapper->m_lSinksFromSrc, srcIdx, &m_pWrapper->m_Node))
                return createIndex(m_pWrapper->m_lSinksFromSrc[srcIdx.row()] - 1, 0, Q_NULLPTR);
            break;
    }

    return {};
}

QModelIndex QNodeEdgeFilterProxy::mapToSource(const QModelIndex& proxyIndex) const
{
    if ((!proxyIndex.isValid()) || proxyIndex.model() != this)
        return {};

    if (proxyIndex.column())
        return {};

    const auto srcP = m_pWrapper->m_Node.index();

    switch (m_Type) {
        case GraphicsNodeSocket::SocketType::SOURCE:
            return d_ptr->q_ptr->index(m_pWrapper->m_lSourcesToSrc[proxyIndex.row()], 0, srcP);
        case GraphicsNodeSocket::SocketType::SINK:
            return d_ptr->q_ptr->index(m_pWrapper->m_lSinksToSrc[proxyIndex.row()], 0, srcP);
    }

    return {};
}

int QNodeEdgeFilterProxy::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 4;
}

QModelIndex QNodeEdgeFilterProxy::index(int row, int column, const QModelIndex& parent) const
{
    if (parent.isValid() || row < 0 || column < 0 || column > 3)
        return {};

    switch(m_Type) {
        case GraphicsNodeSocket::SocketType::SOURCE:
            if (row < m_pWrapper->m_lSources.size())
                return createIndex(row, column, nullptr);
            break;
        case GraphicsNodeSocket::SocketType::SINK:
            if (row < m_pWrapper->m_lSinks.size())
                return createIndex(row, column, nullptr);
            break;
    }

    return {};
}

QVariant QNodeEdgeFilterProxy::data(const QModelIndex& idx, int role) const
{
    if ((!idx.isValid()) || idx.model() != this)
        return {};

    if (!idx.column())
        return QAbstractProxyModel::data(idx, role);

    SocketWrapper* i = Q_NULLPTR;

    switch (m_Type) {
        case GraphicsNodeSocket::SocketType::SOURCE:
            Q_ASSERT(idx.row() < m_pWrapper->m_lSources.size());
            i = m_pWrapper->m_lSources[idx.row()];
            break;
        case GraphicsNodeSocket::SocketType::SINK:
            Q_ASSERT(idx.row() < m_pWrapper->m_lSinks.size());
            i = m_pWrapper->m_lSinks[idx.row()];
            break;
    }

    // There is nothing to display if the socket isn't connected
    if ((!i) || (!i->m_EdgeWrapper))
        return {};

    const auto edgeIdx = i->m_EdgeWrapper->m_Edge.index();

    if (idx.column() == 1) {
        if (role == Qt::DisplayRole)
            return i->m_EdgeWrapper->m_EdgeId;
        else
            return edgeIdx.data(role);
    }

    // This leaves columns 2 and 3 to handle

    if (i->m_EdgeWrapper->m_pSink && i->m_EdgeWrapper->m_pSource) {
        switch (m_Type) {
            case GraphicsNodeSocket::SocketType::SOURCE:
                if (idx.column() == 2)
                    return edgeIdx.model()->index(edgeIdx.row(), 2).data(role);
                else {
                    return i->m_EdgeWrapper->m_pSink->m_pNode->m_Node.index().data(role);
                }
            case GraphicsNodeSocket::SocketType::SINK:
                if (idx.column() == 2)
                    return edgeIdx.model()->index(edgeIdx.row(), 0).data(role);
                else {
                    return i->m_EdgeWrapper->m_pSource->m_pNode->m_Node.index().data(role);
                }
        }
    }

    // If the connection is valid but the sockets are not, then the current
    // state is invalid. All hopes are lost.
    Q_ASSERT(edgeIdx.data(QReactiveProxyModel::ConnectionsRoles::IS_VALID) == false);

    return {};
}

QModelIndex QNodeEdgeFilterProxy::parent(const QModelIndex& idx) const
{
    Q_UNUSED(idx);
    return {};
}

QVariant QNodeEdgeFilterProxy::headerData(int section, Qt::Orientation ori, int role) const
{
    // The default one is good enough
    if (ori == Qt::Vertical || role != Qt::DisplayRole)
        return {};

    static QVariant sock  {QStringLiteral("Socket name")};
    static QVariant edge  {QStringLiteral("Edge")};
    static QVariant other {QStringLiteral("Connected socket")};
    static QVariant node  {QStringLiteral("Connected node")};

    switch(section) {
        case 0:
            return sock;
        case 1:
            return edge;
        case 2:
            return other;
        case 3:
            return node;
    };

    return {};
}

Qt::ItemFlags QNodeEdgeFilterProxy::flags(const QModelIndex &idx) const
{
    if (!idx.isValid())
        return Qt::NoItemFlags;

    if (!idx.column())
        return QAbstractProxyModel::flags(idx);

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

#include <qnodeeditorsocketmodel.moc>
