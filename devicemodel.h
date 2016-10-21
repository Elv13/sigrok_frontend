#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QItemSelectionModel>

#include <memory>

namespace sigrok {
    class HardwareDevice;
    class Context;
}

class DeviceModelPrivate;

class DeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class Role {
        DEVICE_NODE = Qt::UserRole+1
    };

    static DeviceModel* instance();

    Q_INVOKABLE void scan();

    QItemSelectionModel* selectionModel() const;
    std::shared_ptr<sigrok::Context> context() const;

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;

    std::shared_ptr<sigrok::HardwareDevice> currentDevice() const;

Q_SIGNALS:

    void currentDeviceChanged(
        std::shared_ptr<sigrok::HardwareDevice> current,
        std::shared_ptr<sigrok::HardwareDevice> prevous
    );

private:
    explicit DeviceModel();
    virtual ~DeviceModel();
    DeviceModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(DeviceModel);
};

Q_DECLARE_METATYPE(DeviceModel*)
Q_DECLARE_METATYPE(std::shared_ptr<sigrok::HardwareDevice>)
