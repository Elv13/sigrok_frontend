#pragma once

#include <QtWidgets/QWidget>

#include "ui_meter.h"

class Meter : public QWidget, public Ui_Meter
{
    Q_OBJECT

public:
    explicit Meter(QWidget* parent = nullptr);

    virtual ~Meter();
};
