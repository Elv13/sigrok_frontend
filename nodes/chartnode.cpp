#include "chartnode.h"

#include <KChartPlotter>
#include <KChartAbstractProxyModel.h>
#include <KChartAttributesModel.h>
#include <KChartChart>
#include <KChartCartesianCoordinatePlane>
#include <KChartGridAttributes>
#include <KChartHeaderFooter>
#include <KChartLegend>
#include <KChartDataValueAttributes>
#include <KChartBackgroundAttributes>
#include <KChartFrameAttributes.h>

#include "widgets/charttype.h"

#include "common/pagemanager.h"

class ChartNodePrivate : public QObject
{
public:
    KChart::Plotter* m_pPlotter;
    ChartType* m_pChartW {new ChartType};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

ChartNode::ChartNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new ChartNodePrivate())
{
    auto chart = new KChart::Chart(nullptr);
    session()->pages()->addPage(this, chart, title(), uid());

    auto coordinatePlane = dynamic_cast<KChart::CartesianCoordinatePlane*>(
        chart->coordinatePlane()
    );

    d_ptr->m_pPlotter = new KChart::Plotter(chart);
    d_ptr->m_pPlotter->setAntiAliasing(true);
    //plotter->setType(KChart::Plotter::Stacked);

    coordinatePlane->addDiagram(d_ptr->m_pPlotter);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &ChartNodePrivate::slotModelChanged);
}

ChartNode::~ChartNode()
{
    
}

void ChartNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QWidget* ChartNode::widget() const
{
    return d_ptr->m_pChartW;
}

void ChartNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_pPlotter->setModel(newModel);
}
