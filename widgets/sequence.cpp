#include "sequence.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_sequence.h"

Sequence::Sequence(QWidget* parent) : QWidget(parent)
{
    Ui_Sequence ui;
    ui.setupUi(this);

    m_pTable = ui.m_pContent;
    connect(m_pTable->selectionModel(), &QItemSelectionModel::currentChanged,
     [this](const QModelIndex& idx) {
        Q_EMIT currentChanged(idx);
    });
}

Sequence::~Sequence()
{

}

void Sequence::setModel(QAbstractItemModel* m)
{
    m_pTable->setModel(m);
    m_pTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
//     m_pTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
}

void Sequence::slotAdd()
{
    const int r = m_pTable->model()->rowCount();
    m_pTable->model()->insertRows(r,1);
    const auto i = m_pTable->model()->index(r,0);
    m_pTable->selectionModel()->setCurrentIndex(i, QItemSelectionModel::ClearAndSelect);
    m_pTable->setFocus();
    m_pTable->edit(i);
}

void Sequence::slotMoveUp()
{
    const auto cur = m_pTable->selectionModel()->currentIndex();

    if (!cur.isValid())
        return;

    if (!cur.row())
        return;

    m_pTable->model()->moveRow({}, cur.row(), {}, cur.row()-1);
}

void Sequence::slotMoveDown()
{
    const auto cur = m_pTable->selectionModel()->currentIndex();

    if (!cur.isValid())
        return;

    if (cur.row() == m_pTable->model()->rowCount()-1)
        return;

    m_pTable->model()->moveRow({}, cur.row(), {}, cur.row()+1);
}

void Sequence::setCurrent(const QModelIndex& i)
{
    m_pTable->selectionModel()->blockSignals(true);
    m_pTable->selectionModel()->setCurrentIndex(i, QItemSelectionModel::ClearAndSelect);
    m_pTable->selectionModel()->blockSignals(false);
}
