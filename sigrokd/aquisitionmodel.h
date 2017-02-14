#pragma once

#include <memory>

#include <QtCore/QAbstractTableModel>

namespace sigrok {
    class HardwareDevice;
    class Context;
}

class SigrokDevice;

class AquisitionModelPrivate;
class Q_DECL_EXPORT AquisitionModel : public QAbstractTableModel
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
        U_TIMESTAMP
    };

    enum class Mode {
        UNTHROTTLED,
        MANUAL,
    };

    enum class State {
        STOPPED, /* Default                  */
        INIT   , /* Stated, but no data      */
        IDLE   , /* Many sampled failed      */
        TIMEOUT, /* Too many missed samples  */
        STARTED, /* Everything is going well */
        ERROR  , /* There was an error       */
    };
    Q_ENUMS(AquisitionModel::STATE)

    explicit AquisitionModel(SigrokDevice* device);
    virtual ~AquisitionModel();

    void setMode(Mode m);

    int channelCount() const;
    QList<float> currentValues() const;

    bool addLastSample();

    QDateTime lastSampleDateTime() const;

    State state() const;

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

public Q_SLOTS:
    void start();
    void stop();
    void clear();

Q_SIGNALS:
    void currentValuesChanged(QList<float> values);
    void stateChanged(State s, State old);

private:
    AquisitionModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(AquisitionModel)
};
