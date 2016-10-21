#pragma once

#include <QtWidgets/QWidget>

#include "ui_table.h"

class Table : public QWidget, public Ui_Table
{
    Q_OBJECT

public:
    explicit Table(QWidget* parent = nullptr);

    virtual ~Table();
};
