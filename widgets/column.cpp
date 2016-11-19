#include "column.h"

#include <QtCore/QDebug>
#include "ui_column.h"

Column::Column(QWidget* parent) : QWidget(parent)
{
    Ui_Column ui;
    ui.setupUi(this);
    m_pListView = ui.m_pListView;
}

Column::~Column()
{

}

void Column::setModel(QAbstractItemModel* m)
{
    m_pListView->setModel(m);
}
