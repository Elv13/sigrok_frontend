#pragma once

#include <QtWidgets/QWidget>

class QTableView;
class QAbstractItemModel;

class Sequence : public QWidget
{
    Q_OBJECT

public:
    explicit Sequence(QWidget* parent = nullptr);

    virtual ~Sequence();

    void setModel(QAbstractItemModel* m);

public Q_SLOTS:
    void slotAdd();
    void slotMoveUp();
    void slotMoveDown();
    void setCurrent(const QModelIndex& i);

Q_SIGNALS:
    void currentChanged(const QModelIndex& i);

private:
    QTableView* m_pTable;
};
