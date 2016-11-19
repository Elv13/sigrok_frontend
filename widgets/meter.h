#pragma once

#include <QtWidgets/QWidget>

class QTableView;
class QAbstractItemModel;

class Meter : public QWidget
{
    Q_OBJECT

public:
    explicit Meter(QWidget* parent = nullptr);

    virtual ~Meter();

    void setModel(QAbstractItemModel* m);

private:
    QTableView* m_pTable;
};
