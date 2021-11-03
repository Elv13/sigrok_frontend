#include "current.h"
#include <QAbstractItemModel>

// #include "ratesink.h"
#include "ui_current.h"

#include <QtCore/QDebug>

Current::Current(QWidget* parent) : QWidget(parent)
{
    Ui_Current ui;
    ui.setupUi(this);
    m_pValue = ui.m_pValue;
}

Current::~Current()
{

}

void Current::setModel(const QAbstractItemModel* m)
{
//     auto sink = new RateSink(m); //FIXME leak00
//     QObject::connect(sink, &RateSink::rateChangedAsString, m_pRate, &QLabel::setText);

    QObject::connect(m, &QAbstractItemModel::rowsInserted, [m, this]() {
        const auto idx = m->index(m->rowCount()-1, 1);
        m_pValue->setText(idx.data().toString());
    });

}

void Current::setText(const QString& text)
{
    m_pValue->setText(text);
}
