#pragma once

#include <QtWidgets/QWidget>

#include "ui_column.h"

class Column : public QWidget, public Ui_Column
{
    Q_OBJECT

public:
    explicit Column(QWidget* parent = nullptr);

    virtual ~Column();
};
