#pragma once

#include <QtWidgets/QWidget>

class QLCDNumber;
class QAbstractItemModel;

class Q_DECL_EXPORT LCDMeter : public QWidget
{
    Q_OBJECT

public:
    explicit LCDMeter(QWidget* parent = nullptr);

    void setModel(const QAbstractItemModel* m);
    void setValue(float v);
    void setValue(const QModelIndex& idx);

    virtual ~LCDMeter();

private:
    QLCDNumber* m_pValue;
};
