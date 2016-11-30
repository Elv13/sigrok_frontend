#pragma once

#include "../common/device_source.h"

#include <memory>

namespace sigrok {
    class HardwareDevice;
}

class SigrokDevicePrivate;

class Q_DECL_EXPORT SigrokDevice final : public DeviceSource
{
    Q_OBJECT
public:
    explicit SigrokDevice(std::shared_ptr<sigrok::HardwareDevice> d, const QString& name, QObject* parent = nullptr);
    virtual ~SigrokDevice();

    virtual QString name() const override;
    virtual void setName(QString n) override;

    virtual QByteArray type() const override;
    virtual void setType(QByteArray type) override;

    virtual bool isActive() const override;
    virtual void setIsActive(bool a) override;

    std::shared_ptr<sigrok::HardwareDevice> native() const;

public Q_SLOTS:
    void start();
    void stop();

private:
    SigrokDevicePrivate* d_ptr;
    Q_DECLARE_PRIVATE(SigrokDevice)
};

Q_DECLARE_METATYPE(SigrokDevice*)
