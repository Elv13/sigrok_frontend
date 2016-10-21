#pragma once

#include <QtWidgets/QWidget>

#include "ui_aquisition.h"

class AquisitionModel;

class Aquisition : public QWidget, public Ui_Aquisition
{
    Q_OBJECT

public:
    explicit Aquisition(QWidget* parent = nullptr);

    void setModel(const AquisitionModel* m);

    virtual ~Aquisition();
};
