#pragma once

#include "abstractnode.h"

#include <memory>

//#include "devicemodel.h"

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
    Q_PROPERTY(std::shared_ptr<sigrok::HardwareDevice> device WRITE setDevice USER true);
    Q_PROPERTY(bool aboutToClear READ dummy NOTIFY aboutToClear USER true);
    Q_PROPERTY(bool cleared READ dummy NOTIFY cleared USER true);
    Q_PROPERTY(bool clear WRITE clear USER true);
    Q_PROPERTY(bool start WRITE start USER true);
    Q_PROPERTY(bool stop WRITE stop USER true);

    Q_INVOKABLE explicit DeviceNode(QObject* parent = nullptr);
//     DeviceNode(QObject* parent = nullptr);
    virtual ~DeviceNode();

    virtual QString title() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    void setModel(AquisitionModel* m);

    virtual QAbstractItemModel* model() const;

    bool dummy() const;

    void clear(bool value);
    void start(bool value);
    void stop(bool value);

    std::shared_ptr<sigrok::HardwareDevice> device() const;
    void setDevice(std::shared_ptr<sigrok::HardwareDevice> dev);

    virtual QWidget* widget() const;

Q_SIGNALS:
    void modelChanged(); //Dummy
    void aboutToClear();
    void cleared();

private:
    DeviceNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(DeviceNode)

};

Q_DECLARE_METATYPE(DeviceNode*)
