#include "curvechartnode.h"

#include "widgets/aquisition.h"

#include "widgets/curvechart.h"

#include "widgets/charttype.h"

#include "common/pagemanager.h"

class CurveChartNodePrivate : public QObject
{
public:
    CurveChart* m_pPlotter;
//     ChartType* m_pChartW {new ChartType};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

CurveChartNode::CurveChartNode(QObject* parent) : ProxyNode(parent), d_ptr(new CurveChartNodePrivate())
{
//     auto chart = new KChart::Chart(nullptr);

    d_ptr->m_pPlotter = new CurveChart();

    PageManager::instance()->addPage(d_ptr->m_pPlotter, "Chart");

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
