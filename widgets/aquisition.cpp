#include "aquisition.h"

// #include "ratesink.h"
// #include "aquisitionmodel.h"

#include <QtCore/QDebug>

#include "ui_aquisition.h"

Aquisition::Aquisition(QWidget* parent) : QWidget(parent)
{
    Ui_Aquisition aq;
    aq.setupUi(this);
}

Aquisition::~Aquisition()
{

}

void Aquisition::setModel(const AquisitionModel* m)
{
    Q_UNUSED(m)
//     auto sink = new RateSink(m); //FIXME leak

//     QObject::connect(sink, &RateSink::rateChangedAsString, m_pRate, &QLabel::setText);
// 
//     QObject::connect(m, &AquisitionModel::currentValuesChanged, [this](QList<float> values) {
//         m_pValue->setText(QString::number(values[0]));
//     });

}

void Aquisition::slotStarted()
{
    Q_EMIT started();
}

void Aquisition::slotStopped()
{
    Q_EMIT stopped();
}

void Aquisition::slotCleared()
{
    Q_EMIT cleared();
}
