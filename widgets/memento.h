#pragma once

#include <QtWidgets/QWidget>

#include "ui_memento.h"

class Memento : public QWidget, public Ui_Memento
{
    Q_OBJECT

public:
    explicit Memento(QWidget* parent = nullptr);

    virtual ~Memento();
};
