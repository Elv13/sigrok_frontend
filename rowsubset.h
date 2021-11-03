#pragma once

#include <QtWidgets/QWidget>

class RowSubset : public QWidget
{
    Q_OBJECT

public:
    explicit RowSubset(QWidget* parent = nullptr);
    virtual ~RowSubset();

public Q_SLOTS:
    void slotLimit(bool v);
    void slotMaxRow(int r);

Q_SIGNALS:
    void limitChanged(bool);
    void maxRowChanged(int);
};
