#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class DeviceListNodePrivate;

class Q_DECL_EXPORT DeviceListNode : public AbstractNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit DeviceListNode(AbstractSession* sess);
    virtual ~DeviceListNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}
    virtual QAbstractItemModel* sourceModel() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

Q_SIGNALS:
    void activated(bool);
    void tick(bool);

private:
    DeviceListNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(DeviceListNode)

};
Q_DECLARE_METATYPE(DeviceListNode*)
