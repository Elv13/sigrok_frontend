#include "plotnode.h"

#include "common/pagemanager.h"
#include "common/abstractsession.h"

#include "proxies/vectorizingproxy.h"

#include "remotemanager.h"
#include "widgets/plot.h"

#include <QtCore/QTimer>
#include <QtCore/QIdentityProxyModel>

class QwtPlotNodePrivate : public QObject
{
    Q_OBJECT
public:
    Plot* m_pPlot {new Plot()};
    mutable QIdentityProxyModel* m_pRemoteModel {nullptr};
    mutable QString m_Id;

    QwtPlotNode* q_ptr;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

QwtPlotNode::QwtPlotNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new QwtPlotNodePrivate())
{
    d_ptr->q_ptr = this;

    QTimer::singleShot(0, [this](){
        session()->pages()->addPage(this, d_ptr->m_pPlot, title(), uid());
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

    m_pPlot->setModel(newModel);

    if (m_pRemoteModel) {
        m_pRemoteModel->setSourceModel(newModel);
    }
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
    d_ptr->m_pPlot->replot();
}

#include <plotnode.moc>
