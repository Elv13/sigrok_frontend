#pragma once

#include <QtWidgets/QWidget>

#include "ui_devicelist.h"

class DeviceList : public QWidget, public Ui_DeviceList
{
    Q_OBJECT

public:
    explicit DeviceList(QWidget* parent = nullptr);

    virtual ~DeviceList();
};
