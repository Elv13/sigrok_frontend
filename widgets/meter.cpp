#include "meter.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_meter.h"

Meter::Meter(QWidget* parent) : QWidget(parent)
{
    Ui_Meter ui;
    ui.setupUi(this);

    m_pTable = ui.m_pContent;
}

Meter::~Meter()
{

}

void Meter::setModel(QAbstractItemModel* m)
{
    m_pTable->setModel(m);
    m_pTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_pTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
}
