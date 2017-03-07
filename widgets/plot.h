#pragma once

#include <QtWidgets/QWidget>

class QTableView;
class QAbstractItemModel;

class QwtPlotCurve;
class QwtPlot;
class QwtPlotGrid;
class VectorizingProxy;

class Plot : public QWidget
{
    Q_OBJECT

public:
    explicit Plot(QWidget* parent = nullptr);

    virtual ~Plot();

    void setModel(QAbstractItemModel* m);

public Q_SLOTS:
    void replot();

private:
    QwtPlotCurve* m_pCurve;
    QwtPlot* m_pPlotter;
    QwtPlotGrid* m_pGrid;
    VectorizingProxy* m_pCache;
};
