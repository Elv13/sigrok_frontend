#pragma once

#include <QtWidgets/QWidget>

#include "ui_charttype.h"

class ChartType : public QWidget, public Ui_ChartType
{
    Q_OBJECT

public:
    explicit ChartType(QWidget* parent = nullptr);

    virtual ~ChartType();
};
