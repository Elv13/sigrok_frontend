#include "statedialog.h"

#include <QtWidgets/QTableView>
#include <QtWidgets/QStyledItemDelegate>
#include <QtWidgets/QComboBox>
#include "ui_statedialog.h"

class CellEditor : public QStyledItemDelegate
{
public:
    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    QAbstractItemModel* m_pParentModel {nullptr};
};

QWidget* CellEditor::createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (!m_pParentModel)
        return nullptr;

    auto ret = new QComboBox(parent);

    ret->setModel(m_pParentModel);

    return ret;
}

void CellEditor::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    if (auto ed = qobject_cast<QComboBox*>(editor))
        ed->setCurrentText(index.data().toString());
}

void CellEditor::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    const auto currentText = qobject_cast<QComboBox*>(editor)->currentText();

    // slow, but rare
    for (int i =0; i < m_pParentModel->rowCount(); i++) {
        const QString s = m_pParentModel->index(i, 0).data().toString();
        if (s == currentText) {
            model->setData(index, s, Qt::DisplayRole);
            model->setData(index, s, Qt::EditRole);
            model->setData(index, i, Qt::UserRole);
        }
    }
}

StateDialog::StateDialog(QWidget* parent) : QDialog(parent)
{
    m_pDelegate = new CellEditor();

    Ui_StateDialog ui;
    ui.setupUi(this);

    m_pActions = ui.m_pActions;
    m_pStates  = ui.m_pStates;
    m_pMatrix  = ui.m_pMatrix;
    m_pMatrix->setItemDelegate(m_pDelegate);
}

StateDialog::~StateDialog()
{
    delete m_pDelegate;
}

void StateDialog::slotAddState()
{
    m_pStateModel->insertRow(m_pStateModel->rowCount());
    m_pStates->setFocus(Qt::OtherFocusReason);
    const auto i = m_pStateModel->index(m_pStateModel->rowCount()-1,0);
    m_pStates->selectionModel()->setCurrentIndex(i, QItemSelectionModel::ClearAndSelect);
    m_pStates->edit(i);
}

void StateDialog::slotAddAction()
{
    m_pActionsModel->insertRow(m_pActionsModel->rowCount());
    m_pActions->setFocus(Qt::OtherFocusReason);
    const auto i = m_pActionsModel->index(m_pActionsModel->rowCount()-1,0);
    m_pActions->selectionModel()->setCurrentIndex(i, QItemSelectionModel::ClearAndSelect);
    m_pActions->edit(i);
}

void StateDialog::setActionsModel(QAbstractItemModel* m)
{
    m_pActions->setModel(m);
    m_pActionsModel = m;
}

void StateDialog::setStateModel(QAbstractItemModel* m)
{
    m_pStates->setModel(m);
    m_pStateModel = m;
    m_pDelegate->m_pParentModel = m;
}

void StateDialog::setMatrixModel(QAbstractItemModel* m)
{
    m_pMatrix->setModel(m);
    m_pMatrixModel = m;
}
