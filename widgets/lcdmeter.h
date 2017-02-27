#pragma once

#include <QtWidgets/QWidget>

class QLCDNumber;
class QAbstractItemModel;

class Q_DECL_EXPORT LCDMeter : public QWidget
{
    Q_OBJECT

public:
    explicit LCDMeter(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* m);
    void setValue(float v);
    void setValue(const QModelIndex& idx);

    virtual ~LCDMeter();

private Q_SLOTS:
    void applyValue();
    void applyValueChange(const QModelIndex&, const QModelIndex&);

private:
    QLCDNumber* m_pValue;
    bool m_IsColored {false};
    QAbstractItemModel* m_pModel {nullptr};
};
