#pragma once

#include <memory>

#include <QtCore/QAbstractTableModel>

namespace sigrok {
    class HardwareDevice;
    class Context;
}

class AquisitionModelPrivate;

class AquisitionModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class Role {
        IS_CHANNEL = Qt::UserRole+1,
        CHANNEL_NAME,
        CHANNEL_ENABLED,
        UNIT,
        UNIT_NAME,
        QUANTITY,
        QUANTITY_NAME,
    };

    explicit AquisitionModel(std::shared_ptr<sigrok::Context> ctx, std::shared_ptr<sigrok::HardwareDevice> device);
    virtual ~AquisitionModel();

    int channelCount() const;
    QList<float> currentValues() const;

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
    void start();
    void stop();

Q_SIGNALS:
    void currentValuesChanged(QList<float> values);

private:
    AquisitionModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(AquisitionModel)
};
