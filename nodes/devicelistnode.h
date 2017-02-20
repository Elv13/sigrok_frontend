#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class DeviceListNodePrivate;

class Q_DECL_EXPORT DeviceListNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("devicelist_node", "Local device list", "", /*Tags:*/
        "hardware"
    )
public:

    Q_INVOKABLE explicit DeviceListNode(AbstractSession* sess);
    virtual ~DeviceListNode();

    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}
    virtual QAbstractItemModel* sourceModel() const override;

    virtual void write(QJsonObject &parent) const override;

Q_SIGNALS:
    void activated(bool);
    void tick(bool);

private:
    DeviceListNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(DeviceListNode)

};
Q_DECLARE_METATYPE(DeviceListNode*)
