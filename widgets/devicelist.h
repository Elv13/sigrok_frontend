#pragma once

#include <QtWidgets/QWidget>


class DeviceList : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceList(QWidget* parent = nullptr);

    virtual ~DeviceList();
};
