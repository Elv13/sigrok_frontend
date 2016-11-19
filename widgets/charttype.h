#pragma once

#include <QtWidgets/QWidget>

class ChartType : public QWidget
{
    Q_OBJECT

public:
    explicit ChartType(QWidget* parent = nullptr);

    virtual ~ChartType();
};
