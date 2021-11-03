#include "statemachine.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_state.h"

#include "statedialog.h"

StateMachine::StateMachine(QWidget* parent) : QWidget(parent)
{
    Ui_StateMachine ui;
    ui.setupUi(this);
}

StateMachine::~StateMachine()
{

}

void StateMachine::slotSettings()
{
    auto x = new StateDialog();
    x->setActionsModel(m_pActionsModel);
    x->setStateModel  (m_pStateModel  );
    x->setMatrixModel (m_pMatrixModel );
    x->show();
}

void StateMachine::slotOpen()
{

}

void StateMachine::setActionsModel(QAbstractItemModel* m)
{
    m_pActionsModel = m;
}

void StateMachine::setStateModel(QAbstractItemModel* m)
{
    m_pStateModel = m;
}

void StateMachine::setMatrixModel(QAbstractItemModel* m)
{
    m_pMatrixModel = m;
}
