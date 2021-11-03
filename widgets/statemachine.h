#pragma once

#include <QtWidgets/QWidget>

class QAbstractItemModel;

class StateMachine : public QWidget
{
    Q_OBJECT

public:
    explicit StateMachine(QWidget* parent = nullptr);

    virtual ~StateMachine();

    void setActionsModel (QAbstractItemModel* m);
    void setStateModel   (QAbstractItemModel* m);
    void setMatrixModel  (QAbstractItemModel* m);

private Q_SLOTS:
    void slotSettings();
    void slotOpen();

private:
    QAbstractItemModel* m_pActionsModel {nullptr};
    QAbstractItemModel* m_pStateModel   {nullptr};
    QAbstractItemModel* m_pMatrixModel  {nullptr};
};
