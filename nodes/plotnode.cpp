#include "plotnode.h"

#include <qwt6/qwt_plot_curve.h>
#include <qwt6/qwt_plot.h>
#include <qwt6/qwt_plot_grid.h>

#include "common/pagemanager.h"
#include "common/abstractsession.h"

#include "proxies/vectorizingproxy.h"

#include "remotemanager.h"

#include <QtCore/QTimer>
#include <QtCore/QIdentityProxyModel>

class QwtPlotNodePrivate : public QObject
{
    Q_OBJECT
public:
    QwtPlotCurve* m_pCurve {new QwtPlotCurve};
    QwtPlot* m_pPlotter {new QwtPlot()};
    QwtPlotGrid* m_pGrid {new QwtPlotGrid()};
    VectorizingProxy* m_pCache {new VectorizingProxy(this)};
    mutable QIdentityProxyModel* m_pRemoteModel {nullptr};
    mutable QString m_Id;

    QwtPlotNode* q_ptr;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

QwtPlotNode::QwtPlotNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new QwtPlotNodePrivate())
{
    d_ptr->q_ptr = this;

    d_ptr->m_pCache->setColumns({1});
//     d_ptr->m_pPlotter->attach(d_ptr->m_pCurve);
    d_ptr->m_pCurve->attach(d_ptr->m_pPlotter);
    d_ptr->m_pGrid->attach(d_ptr->m_pPlotter);

    d_ptr->m_pCurve->setPen (Qt::yellow, 2, Qt::SolidLine);

    QTimer::singleShot(0, [this](){
        session()->pages()->addPage(this, d_ptr->m_pPlotter, title(), uid());
    });

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &QwtPlotNodePrivate::slotModelChanged);
}

QwtPlotNode::~QwtPlotNode()
{
    delete d_ptr;
}

void QwtPlotNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

}

QWidget* QwtPlotNode::widget() const
{
    return nullptr;
}

void QwtPlotNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_pCache->setSourceModel(newModel);

    q_ptr->replot();
}

QString QwtPlotNode::remoteModelName() const
{
    if (!d_ptr->m_pRemoteModel) {
        static int count = 1;
        d_ptr->m_Id = id()+QString::number(count++);

        //TODO sync the raw data
        d_ptr->m_pRemoteModel = new QIdentityProxyModel(const_cast<QwtPlotNode*>(this));
        d_ptr->m_pRemoteModel->setSourceModel(model());

        RemoteManager::instance()->addModel(d_ptr->m_pRemoteModel, {
            Qt::DisplayRole,
            Qt::EditRole,
        }, d_ptr->m_Id);
    }

    return d_ptr->m_Id;
}

QString QwtPlotNode::remoteWidgetType() const
{
    return id();
}

void QwtPlotNode::replot(bool)
{
    auto x = d_ptr->m_pCache->rowsVector();
    auto y = d_ptr->m_pCache->sharedVectorData(1).get();

    d_ptr->m_pCurve->setSamples(x.data(), y->data(), std::min(x.size(), y->size()));
    d_ptr->m_pPlotter->replot();
}

#include <plotnode.moc>
