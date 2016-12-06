#pragma once

#include <QtWidgets/QWidget>

class QAbstractItemModel;

class CurveChart : public QWidget
{
    Q_OBJECT

public:
    explicit CurveChart(QWidget* parent = nullptr);

    virtual ~CurveChart();

    void setModel(QAbstractItemModel* m);

    virtual QSize sizeHint() const override;

protected:
    virtual void paintEvent(QPaintEvent *event) override;

private Q_SLOTS:
    void slotRowsInserted(const QModelIndex&, int, int);

private:
    QAbstractItemModel* m_pModel {Q_NULLPTR};
    float m_Min = 99999999999999999;
    float m_Max = 0;
};
