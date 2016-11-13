#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class DeviceListNodePrivate;

class DeviceListNode : public AbstractNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit DeviceListNode(QObject* parent = nullptr);
    virtual ~DeviceListNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual Mode mode() const {return AbstractNode::Mode::MODEL;}
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
