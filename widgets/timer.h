#pragma once

#include <QtWidgets/QWidget>

#include "ui_timer.h"

class Timer : public QWidget, public Ui_Timer
{
    Q_OBJECT

public:
    explicit Timer(QWidget* parent = nullptr);

    virtual ~Timer();
};
