#include "lcdmeter.h"

// #include "ratesink.h"
#include "ui_lcdmeter.h"

#include "sigrokd/acquisitionmodel.h"

#include <QtCore/QDebug>

LCDMeter::LCDMeter(QWidget* parent) : QWidget(parent)
{
    Ui_LCDMeter ui;
    ui.setupUi(this);
    m_pValue = ui.m_pValue;
    setStyleSheet(QStringLiteral("background-color:#dddddd;color:black;"));
}

LCDMeter::~LCDMeter()
{

}

void LCDMeter::setModel(QAbstractItemModel* m)
{
    if (m_pModel) {
        disconnect(m, &QAbstractItemModel::rowsInserted , this, &LCDMeter::applyValue);
        disconnect(m, &QAbstractItemModel::modelReset   , this, &LCDMeter::applyValue);
        disconnect(m, &QAbstractItemModel::layoutChanged, this, &LCDMeter::applyValue);
        disconnect(m, &QAbstractItemModel::dataChanged  , this, &LCDMeter::applyValue);
    }

    m_pModel = m;

    if (!m)
        return;

    applyValue();

    connect(m, &QAbstractItemModel::rowsInserted , this, &LCDMeter::applyValue);
    connect(m, &QAbstractItemModel::modelReset   , this, &LCDMeter::applyValue);
    connect(m, &QAbstractItemModel::layoutChanged, this, &LCDMeter::applyValue);
    connect(m, &QAbstractItemModel::dataChanged  , this, &LCDMeter::applyValue);

}

void LCDMeter::applyValue()
{
    if (!m_pModel)
        return;

    if (m_pModel->rowCount())
        setValue(m_pModel->index(m_pModel->rowCount()-1, 1));
}

void LCDMeter::applyValueChange(const QModelIndex&, const QModelIndex& br)
{
    if (!m_pModel)
        return;

    if (br.row() == m_pModel->rowCount() -1)
        setValue(m_pModel->index(m_pModel->rowCount()-1, 1));
}

void LCDMeter::setValue(float v)
{
    m_pValue->display(v);
}

void LCDMeter::setValue(const QModelIndex& idx)
{
    if (!idx.isValid())
        return;


    const auto dt = idx.data();
    if (dt.canConvert<float>())
        m_pValue->display(dt.toFloat());

    const auto fg = idx.data(Qt::ForegroundRole);
    const auto bg = idx.data(Qt::BackgroundRole);

    const bool wasValid = m_IsColored;
    m_IsColored = fg.isValid() || bg.isValid();

    if (wasValid && !m_IsColored) {
        setStyleSheet(QStringLiteral("background-color:#dddddd;color:black;"));
        return;
    }

    if (m_IsColored) {
        const auto bgCol = qvariant_cast<QColor>(bg);
        const auto fgCol = qvariant_cast<QColor>(fg);

        setStyleSheet(QString("%1%2")
            .arg(
                bgCol.isValid() ? QString("background-color:%1;")
                    .arg(bgCol.name()) : QString()
            ).arg(
                fgCol.isValid() ? QString("color:%1;")
                    .arg(fgCol.name()) : QString()
            )
        );
    }
}
