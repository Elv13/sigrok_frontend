#include "rowsubset.h"

#include <QtCore/QDebug>
#include "ui_rowsubset.h"

RowSubset::RowSubset(QWidget* parent) : QWidget(parent)
{
    Ui_RowSubset ui;
    ui.setupUi(this);
}

RowSubset::~RowSubset()
{

}

void RowSubset::slotLimit(bool v)
{
    Q_EMIT limitChanged(v);
}

void RowSubset::slotMaxRow(int r)
{
    Q_EMIT maxRowChanged(r);
}
