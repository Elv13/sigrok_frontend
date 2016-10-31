#pragma once

#include <QtWidgets/QWidget>
#include <functional>

#include "ui_range.h"

class RangeProxy;

class Range : public QWidget, public Ui_Range
{
    Q_OBJECT

public:
    explicit Range(QWidget* parent = nullptr);
    virtual ~Range();

    RangeProxy* rangeProxy() const;
    void setRangeProxy(RangeProxy* p);

    void setColumnWidgetFactory(int col, bool isRange, std::function<QWidget*(int)> w);

private:
    RangeProxy* m_pProxy {nullptr};
    QVector< std::function<QWidget*(int)> > m_lWidgetFactories; //FIXME deadcode
    QVector< std::function<QWidget*(int)> > m_lWidgetFactoriesChild;

    // Helpers
    void applyWidget(const QModelIndex& root, QVector< std::function<QWidget*(int)> >& f);

private Q_SLOTS:
    void slotAjustColumns();
};
