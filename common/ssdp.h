#pragma once

#include <QtCore/QAbstractListModel>



namespace ssdp {

class SSDPPrivate;
class Service;
class DevicePrivate;

class Device
{
public:
    enum class State {
        PAIRED,
        UNREACHABLE
    };

    enum class Type {
        LOCAL,
        REMOTE,
    };

    int ping() const;

    Device::State state() const;
    QString uniqueServiceName() const;
    QString location() const;
    long long update_time() const;

private:
    friend class Service;
    explicit Device(Service* s);
    DevicePrivate* d_ptr;
    Q_DECLARE_PRIVATE(Device)
};

class Q_DECL_EXPORT SSDP : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        LOCATION = Qt::UserRole+1,
        PORT,
        IP,
    };
    Q_ENUMS(Role)

    Q_PROPERTY(bool autoBroadcast READ isAutoBroadcast WRITE setAutoBroadcast NOTIFY autoBroadcastChanged)

    explicit SSDP(QObject* parent = nullptr);
    virtual ~SSDP();

    Q_INVOKABLE virtual QVariant data(const QModelIndex& idx, int role) const override;
    Q_INVOKABLE virtual int rowCount(const QModelIndex& parent = {}) const override;
    Q_INVOKABLE virtual QHash<int, QByteArray> roleNames() const override;


    Q_INVOKABLE void searchNow();

    void setAutoBroadcast(bool value);
    bool isAutoBroadcast() const;

    Device* deviceAt(const QModelIndex& idx) const;

Q_SIGNALS:
    void deviceAdded(Device* device);
    void autoBroadcastChanged(bool);

private:
    SSDPPrivate* d_ptr;
    Q_DECLARE_PRIVATE(SSDP)
};

}

Q_DECLARE_METATYPE(ssdp::Device*)
