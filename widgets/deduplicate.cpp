#include "deduplicate.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_deduplicate.h"

#include "nodes/deduplicatenode.h"

#include "proxies/columnproxy.h"

Deduplicate::Deduplicate(const DeduplicateNode* n, QWidget* parent) : QWidget(parent)
{
    Ui_Deduplicate ui;
    ui.setupUi(this);
    m_pColumnCBB = ui.comboBox;

    m_CProxy = new ColumnProxy(this);
    ui.comboBox->setModel(m_CProxy);

    ui.m_pAddColumn->setChecked(n->hasExtraColumn());
    ui.m_pThreshold->setValue(n->threshold());

    connect(m_CProxy, &QAbstractItemModel::rowsInserted, this, &Deduplicate::slotColumnInserted);
    connect(m_pColumnCBB, SIGNAL(currentIndexChanged(int)), this, SLOT(slotIndexChanged()));
}

Deduplicate::~Deduplicate()
{

}

void Deduplicate::slotEnableExtraColumn(bool v)
{
    Q_EMIT enableExtraColumn(v);
}

void Deduplicate::slotChangeTreshold(double value)
{
    Q_EMIT thresholdChanged(value);
}

void Deduplicate::slotIndexChanged(int idx)
{
    Q_EMIT currentColumnChanged(idx);
}

void Deduplicate::setModel(QAbstractItemModel* m)
{
    m_CProxy->setSourceModel(m);
    slotColumnInserted();
}

QString Deduplicate::selectedColumnName() const
{
    return m_pColumnCBB->currentText();
}

void Deduplicate::setPreferredColumn(const QString& name)
{
    m_PreferredColumn = name;
}

void Deduplicate::slotColumnInserted()
{
    if (m_HasManualSelection)
        return;

    for (int i=0; i < m_CProxy->rowCount(); i++) {
        if (m_CProxy->index(i,0).data() == m_PreferredColumn) {
            m_pColumnCBB->setCurrentIndex(i);
            break;
        }
    }

    if (m_pColumnCBB->currentIndex() == -1)
        m_pColumnCBB->setCurrentIndex(m_CProxy->rowCount()-1);
}

void Deduplicate::slotIndexChanged()
{
    m_HasManualSelection = true;
}
