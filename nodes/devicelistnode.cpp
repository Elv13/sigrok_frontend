#include "devicelistnode.h"

#include <QtCore/QDebug>

#include "sigrokd/devicemodel.h"

class DeviceListNodePrivate final
{
public:
    DeviceListNode* q_ptr;
};

DeviceListNode::DeviceListNode(QObject* parent) : AbstractNode(parent), d_ptr(new DeviceListNodePrivate())
{
}

DeviceListNode::~DeviceListNode()
{
    delete d_ptr;
}

QString DeviceListNode::title() const
{
    return "Local device list";
}

QString DeviceListNode::id() const
{
    return QStringLiteral("devicelist_node");
}

void DeviceListNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* DeviceListNode::widget() const
{
    return nullptr;
}

QAbstractItemModel* DeviceListNode::sourceModel() const
{
    return DeviceModel::instance();
}