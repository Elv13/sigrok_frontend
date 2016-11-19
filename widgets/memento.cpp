#include "memento.h"

#include <QtCore/QDebug>

#include "ui_memento.h"

Memento::Memento(QWidget* parent) : QWidget(parent)
{
    Ui_Memento ui;
    ui.setupUi(this);

    m_pTable = ui.tableView;
}

Memento::~Memento()
{

}

void Memento::setModel(QAbstractItemModel* m, QItemSelectionModel* s)
{
    m_pTable->setModel(m);
    m_pTable->setSelectionModel(s);

    m_pTable->verticalHeader()->setHidden(true);
    m_pTable->horizontalHeader()->setHidden(true);
    m_pTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
}

void Memento::slotTakeMemento()
{
    Q_EMIT takeMemento(true);
}
