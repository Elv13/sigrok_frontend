#pragma once

#include <QtWidgets/QWidget>

#include "ui_rowsubset.h"

class RowSubset : public QWidget, public Ui_RowSubset
{
    Q_OBJECT

public:
    explicit RowSubset(QWidget* parent = nullptr);

    virtual ~RowSubset();
};
