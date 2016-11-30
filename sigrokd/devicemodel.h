#pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QItemSelectionModel>

#include <memory>

namespace sigrok {
    class HardwareDevice;
    class Context;
}


Q_DECLARE_METATYPE(std::shared_ptr<sigrok::HardwareDevice>)

class DeviceModelPrivate;

class Q_DECL_EXPORT DeviceModel : public QAbstractListModel
{
    Q_OBJECT

public:
    enum class Role {
        DEVICE = Qt::UserRole+1,
    };

    static DeviceModel* instance();

    Q_INVOKABLE void scan();

    QItemSelectionModel* selectionModel() const;
    std::shared_ptr<sigrok::Context> context() const;

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QHash<int, QByteArray> roleNames() const override;

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
    Q_DECLARE_PRIVATE(DeviceModel)
};

Q_DECLARE_METATYPE(DeviceModel*)
