#include "curvechartnode.h"

#include "widgets/aquisition.h"

#include "widgets/curvechart.h"

#include "widgets/charttype.h"

#include "common/pagemanager.h"
#include "common/abstractsession.h"

#include "remotemanager.h"

#include <QtCore/QTimer>
#include <QtCore/QIdentityProxyModel>

class CurveChartNodePrivate : public QObject
{
public:
    CurveChart* m_pPlotter;
//     ChartType* m_pChartW {new ChartType};
    mutable QIdentityProxyModel* m_pRemoteModel {nullptr};
    mutable QString m_Id;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

CurveChartNode::CurveChartNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new CurveChartNodePrivate())
{
//     auto chart = new KChart::Chart(nullptr);

    d_ptr->m_pPlotter = new CurveChart();

    QTimer::singleShot(0, [this](){
        session()->pages()->addPage(this, d_ptr->m_pPlotter, title(), uid());
    });
//     auto coordinatePlane = dynamic_cast<KChart::CartesianCoordinatePlane*>(
//         chart->coordinatePlane()
//     );

// //     d_ptr->m_pPlotter->setAntiAliasing(true);
    //plotter->setType(KChart::Plotter::Stacked);

//     coordinatePlane->addDiagram(d_ptr->m_pPlotter);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &CurveChartNodePrivate::slotModelChanged);
}

CurveChartNode::~CurveChartNode()
{
    
}

QString CurveChartNode::title() const
{
    return "Chart";
}

QString CurveChartNode::id() const
{
    return QStringLiteral("curvedchart_node");
}

void CurveChartNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QWidget* CurveChartNode::widget() const
{
    return nullptr;//d_ptr->m_pChartW;
}

void CurveChartNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_pPlotter->setModel(newModel);
}


QString CurveChartNode::remoteModelName() const
{
    if (!d_ptr->m_pRemoteModel) {
        static int count = 1;
        d_ptr->m_Id = id()+QString::number(count++);

        d_ptr->m_pRemoteModel = new QIdentityProxyModel(const_cast<CurveChartNode*>(this));
        d_ptr->m_pRemoteModel->setSourceModel(model());

        RemoteManager::instance()->addModel(d_ptr->m_pRemoteModel, {
            Qt::DisplayRole,
            Qt::EditRole,
        }, d_ptr->m_Id);
    }

    return d_ptr->m_Id;
}

QString CurveChartNode::remoteWidgetType() const
{
    return id();
}
