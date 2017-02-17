#include "qnodeview.h"

#include <QtCore/QDebug>
#include <QtCore/QItemSelectionModel>

#include "graphicsnode.hpp"

#include "qreactiveproxymodel.h"
#include "graphicsnodescene.hpp"
#include "graphicsnodesocket.hpp"

#include "qnodeeditorsocketmodel.h"

class QNodeViewPrivate final : public QObject
{
    Q_OBJECT
public:
    explicit QNodeViewPrivate(QObject* p) : QObject(p) {}

    QReactiveProxyModel*    m_pProxy  {Q_NULLPTR};
    QAbstractItemModel*     m_pModel  {Q_NULLPTR};
    QVector<GraphicsNode*>  m_lNodes  {         };
    GraphicsNodeScene       m_Scene   {this     };
    QNodeEditorSocketModel* m_pFactory{Q_NULLPTR};

    QNodeView* q_ptr;

public Q_SLOTS:
    void slotSelectionChanged();
};

QNodeView::QNodeView(QWidget* parent) : GraphicsNodeView(parent),
    d_ptr(new QNodeViewPrivate(this))
{
    d_ptr->q_ptr = this;
    d_ptr->m_pProxy = new QReactiveProxyModel(this);
    d_ptr->m_pFactory = new QNodeEditorSocketModel(d_ptr->m_pProxy, &d_ptr->m_Scene);

    m_pModel = d_ptr->m_pFactory; //HACK to remove
    m_pSelectionModel = new QItemSelectionModel(m_pModel);

    connect(&d_ptr->m_Scene, &QGraphicsScene::selectionChanged, d_ptr, &QNodeViewPrivate::slotSelectionChanged);

    setScene(&d_ptr->m_Scene);
}

QNodeView::~QNodeView()
{
    // Delete the proxies in order or they will crash
    delete d_ptr->m_pFactory;
    delete d_ptr->m_pProxy;
    delete d_ptr;
}

QGraphicsScene* QNodeView::scene() const
{
    return &d_ptr->m_Scene;
}

void QNodeView::setModel(QAbstractItemModel* m)
{
    d_ptr->m_pModel = m;
    d_ptr->m_pProxy->setSourceModel(m);
}

GraphicsNode* QNodeView::getNode(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return Q_NULLPTR;

    const auto factoryIdx = d_ptr->m_pFactory->mapFromSource(
        d_ptr->m_pProxy->mapFromSource(idx)
    );

    Q_ASSERT(factoryIdx.isValid());

    return d_ptr->m_pFactory->getNode(factoryIdx);
}

QReactiveProxyModel* QNodeView::reactiveModel() const
{
    return d_ptr->m_pProxy;
}

QAbstractItemModel *QNodeView::sinkSocketModel(const QModelIndex& node) const
{
    return d_ptr->m_pFactory->sinkSocketModel(node);
}

QAbstractItemModel *QNodeView::sourceSocketModel(const QModelIndex& node) const
{
    return d_ptr->m_pFactory->sourceSocketModel(node);
}

QAbstractItemModel* QNodeView::edgeModel() const
{
    return d_ptr->m_pFactory->edgeModel();
}

QItemSelectionModel* QNodeView::selectionModel() const
{
    return m_pSelectionModel;
}

void QNodeView::setSelectionModel(QItemSelectionModel* m)
{
    m_pSelectionModel = m;
    d_ptr->slotSelectionChanged();
}

void QNodeViewPrivate::slotSelectionChanged()
{
    const auto sel = q_ptr->selectedNodeIndexes();

    switch(sel.size()) {
        case 0:
            q_ptr->m_pSelectionModel->clear();
            return;
        case 1:
            q_ptr->m_pSelectionModel->setCurrentIndex(
                sel.first(),
                QItemSelectionModel::Clear  |
                QItemSelectionModel::Select |
                QItemSelectionModel::Current
            );
            return;
        default:
            for (const auto& idx : sel) {
                q_ptr->m_pSelectionModel->setCurrentIndex(
                    idx,
                    QItemSelectionModel::Select
                );
            }
            break;
    }

}

QModelIndex QNodeView::indexAt(const QPoint &point) const
{
    auto n = nodeAtAbs(point);

    return n ? n->index() : QModelIndex();
}

#include <qnodeview.moc>

