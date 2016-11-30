#pragma once

#include <QtWidgets/QWidget>

class QLCDNumber;
class QAbstractItemModel;

class LCDMeter : public QWidget
{
    Q_OBJECT

public:
    explicit LCDMeter(QWidget* parent = nullptr);

    void setModel(const QAbstractItemModel* m);
    void setValue(float v);

    virtual ~LCDMeter();

private:
    QLCDNumber* m_pValue;
};
