#include "charttype.h"

#include "../ratesink.h"
#include "../aquisitionmodel.h"

#include <QtCore/QDebug>

ChartType::ChartType(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

ChartType::~ChartType()
{

}
