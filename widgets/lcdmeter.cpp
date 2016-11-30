#include "lcdmeter.h"

// #include "ratesink.h"
#include "ui_lcdmeter.h"

#include <QtCore/QDebug>

LCDMeter::LCDMeter(QWidget* parent) : QWidget(parent)
{
    Ui_LCDMeter ui;
    ui.setupUi(this);
    m_pValue = ui.m_pValue;
}

LCDMeter::~LCDMeter()
{

}

void LCDMeter::setModel(const QAbstractItemModel* m)
{
//     auto sink = new RateSink(m); //FIXME leak00
//     QObject::connect(sink, &RateSink::rateChangedAsString, m_pRate, &QLabel::setText);

    QObject::connect(m, &QAbstractItemModel::rowsInserted, [m, this]() {
        const auto idx = m->index(m->rowCount()-1, 1);
        m_pValue->display(idx.data().toFloat());
    });

}

void LCDMeter::setValue(float v)
{
    m_pValue->display(v);
}
