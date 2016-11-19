#pragma once

#include <QtWidgets/QWidget>

class QTableView;
class QAbstractItemModel;
class QItemSelectionModel;

class Memento : public QWidget
{
    Q_OBJECT

public:
    explicit Memento(QWidget* parent = nullptr);

    virtual ~Memento();

    void setModel(QAbstractItemModel* m, QItemSelectionModel* s);

public Q_SLOTS:
    void slotTakeMemento();

Q_SIGNALS:
    void takeMemento(bool);

private:
    QTableView* m_pTable;
};
