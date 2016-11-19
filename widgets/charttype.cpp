#include "charttype.h"

#include "ui_charttype.h"

#include <QtCore/QDebug>

ChartType::ChartType(QWidget* parent) : QWidget(parent)
{
    Ui_ChartType ui;
    ui.setupUi(this);
}

ChartType::~ChartType()
{

}
