#pragma once

#include <QtWidgets/QWidget>

#include "ui_current.h"

class QAbstractItemModel;

class Current : public QWidget, public Ui_Current
{
    Q_OBJECT

public:
    explicit Current(QWidget* parent = nullptr);

    void setModel(const QAbstractItemModel* m);

    virtual ~Current();
};
