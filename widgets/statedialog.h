#pragma once

#include <QtWidgets/QDialog>

class QAbstractItemModel;
class QListView;
class QTableView;

class CellEditor;

class StateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit StateDialog(QWidget* parent = nullptr);

    void setActionsModel (QAbstractItemModel* m);
    void setStateModel   (QAbstractItemModel* m);
    void setMatrixModel  (QAbstractItemModel* m);

    virtual ~StateDialog();

public Q_SLOTS:
    void slotAddState();
    void slotAddAction();

private:
    QAbstractItemModel* m_pActionsModel {nullptr};
    QAbstractItemModel* m_pStateModel   {nullptr};
    QAbstractItemModel* m_pMatrixModel  {nullptr};
    QListView*  m_pActions;
    QListView*  m_pStates;
    QTableView* m_pMatrix;
    CellEditor* m_pDelegate;
};
