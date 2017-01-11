#include "controls.h"

// #include "ratesink.h"
#include "ui_controls.h"

#include <QtWidgets/QPushButton>

#include <QtCore/QDebug>

Controls::Controls(QWidget* parent) : QWidget(parent)
{
    Ui_Controls ui;
    ui.setupUi(this);
    m_pTable = ui.m_pTable;
}

Controls::~Controls()
{

}

void Controls::setModel(QAbstractItemModel* m)
{
    if (m_pModel)
        disconnect(m_pModel, &QAbstractItemModel::rowsInserted, this, &Controls::slotRowsInserted);

    m_pTable->setModel(m);

    m_pModel = m;

    if (!m)
        return;

    connect(m_pModel, &QAbstractItemModel::rowsInserted, this, &Controls::slotRowsInserted);

    slotRowsInserted({}, 0, m_pModel->rowCount());

    connect(m_pModel, &QAbstractItemModel::modelReset, this, &Controls::slotModelReset);
    connect(m_pModel, &QAbstractItemModel::layoutChanged, this, &Controls::slotModelReset);
    connect(m_pModel, &QAbstractItemModel::dataChanged, this, &Controls::slotDataChanged);
}

void Controls::slotRowsInserted(const QModelIndex&, int start, int end)
{
    for (int i = start; i <= end; i++) {
        const QPersistentModelIndex idx = m_pModel->index(i, 0);

        auto pb = new QPushButton(idx.data().toString());
        connect(pb, &QPushButton::clicked, [idx, this]() {
            m_pModel->setData(idx, true, Qt::EditRole);
        });

        m_pTable->setIndexWidget(idx, pb);
    }
}

void Controls::slotModelReset()
{
    slotRowsInserted({}, 0, m_pModel->rowCount()-1);
}

void Controls::slotDataChanged(const QModelIndex& tl, const QModelIndex& br)
{
    for (int i = tl.row(); i <= br.row(); i++) {
        const QModelIndex& idx = m_pModel->index(i, 0);
        if (auto w = m_pTable->indexWidget(idx)) {
            if (auto pb = qobject_cast<QPushButton*>(w)) {
                pb->setText(idx.data().toString());
            }
        }
        else {
            slotRowsInserted({}, i, i);
        }
    }
}
