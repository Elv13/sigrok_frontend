#pragma once

#include "abstractnode.h"

#include <memory>

#include <QtCore/QObject>

#include <QtCore/QAbstractItemModel>

class AquisitionModel;

class DeviceNodePrivate;

namespace sigrok {
    class HardwareDevice;
}

class DeviceNode : public AbstractNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY modelChanged USER true);

    explicit DeviceNode(QObject* parent = nullptr);
    DeviceNode(const QString &name, std::shared_ptr<sigrok::HardwareDevice> dev, QObject* parent = nullptr);
    virtual ~DeviceNode();

    virtual QString title() const;

    void setModel(AquisitionModel* m);

    virtual QAbstractItemModel* model() const;

//     std::shared_ptr<sigrok::HardwareDevice> device() const;
//     void setDevice(std::shared_ptr<sigrok::HardwareDevice> dev);

    virtual QWidget* widget() const;

Q_SIGNALS:
    void modelChanged(); //Dummy

private:
    DeviceNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(DeviceNode)

};

Q_DECLARE_METATYPE(DeviceNode*)
