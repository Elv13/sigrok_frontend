#include "plot.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include <QtWidgets/QHBoxLayout>

#include "proxies/vectorizingproxy.h"
#include <qwt6/qwt_plot_curve.h>
#include <qwt6/qwt_plot.h>
#include <qwt6/qwt_plot_grid.h>

Plot::Plot(QWidget* parent) : QWidget(parent) ,m_pCurve (new QwtPlotCurve),
m_pPlotter (new QwtPlot()), m_pGrid (new QwtPlotGrid()),
m_pCache (new VectorizingProxy(this))
{
    m_pCache->setColumns({1});
    m_pCurve->attach(m_pPlotter);
    m_pGrid->attach(m_pPlotter);

    m_pCurve->setPen (Qt::yellow, 2, Qt::SolidLine);

    auto l = new QHBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->addWidget(m_pPlotter);

    connect(m_pCache, &QAbstractItemModel::rowsInserted , this, &Plot::replot);
    connect(m_pCache, &QAbstractItemModel::rowsRemoved  , this, &Plot::replot);
    connect(m_pCache, &QAbstractItemModel::dataChanged  , this, &Plot::replot);
    connect(m_pCache, &QAbstractItemModel::modelReset   , this, &Plot::replot);
    connect(m_pCache, &QAbstractItemModel::layoutChanged, this, &Plot::replot);
}

Plot::~Plot()
{

}

void Plot::setModel(QAbstractItemModel* m)
{
    m_pCache->setSourceModel(m);

    if (m)
        replot();
}

void Plot::replot()
{
    const auto x = m_pCache->rowsVector();
    const auto y = m_pCache->sharedVectorData(1).get();

    m_pCurve->setSamples(x.data(), y->data(), std::min(x.size(), y->size()));
    m_pPlotter->replot();
}
