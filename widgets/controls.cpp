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
        disconnect(m_pModel, &QAbstractItemModel::rowsInserted, this, &Controls::slowRowsInserted);

    m_pTable->setModel(m);

    m_pModel = m;

    if (!m)
        return;

    connect(m_pModel, &QAbstractItemModel::rowsInserted, this, &Controls::slowRowsInserted);

    slowRowsInserted({}, 0, m_pModel->rowCount());
}

void Controls::slowRowsInserted(const QModelIndex&, int start, int end)
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
