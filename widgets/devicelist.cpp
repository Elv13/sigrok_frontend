#include "devicelist.h"

#include <QtCore/QDebug>
#include "ui_devicelist.h"

DeviceList::DeviceList(QWidget* parent) : QWidget(parent)
{
    Ui_DeviceList ui;
    ui.setupUi(this);
}

DeviceList::~DeviceList()
{

}
