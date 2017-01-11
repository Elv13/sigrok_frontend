#include "aquisitionmodel.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <chrono>

#include "sigrokdevice.h"
#include "devicemodel.h"

struct Chan {
    bool  m_IsEnabled;
    float m_AnalogValue;
};

struct Row {
    int m_Index;
    unsigned long long m_TimeStamp;
    QVector<Chan*> m_lChans;
    qint64 m_Epoch;
    QString m_Unit;
    QString m_Quantity;
    QVariant m_Id;

    //Helpers
    QVariant commonRole(int role) const;
};

class AquisitionModelPrivate
{
public:
    enum Column {
        ID,
        CHANS
    };

    QVector<Row*> m_lRows;
    Row* m_pLastValue {nullptr};
    AquisitionModel::Mode m_Mode {AquisitionModel::Mode::UNTHROTTLED};
    std::shared_ptr<sigrok::HardwareDevice> m_pDev;
    std::shared_ptr<sigrok::Session> m_pSess;
    std::shared_ptr<sigrok::Context> m_pContext;

    void initialize(AquisitionModel* self);
};

void AquisitionModelPrivate::initialize(AquisitionModel* self)
{

    auto analog_datafeed_callback = [self,this](
        std::shared_ptr<sigrok::Device> dev,
        std::shared_ptr<sigrok::Packet> packet,
        std::shared_ptr<sigrok::Analog> analog
    ) {
        Q_UNUSED(dev)
        Q_UNUSED(packet)
        if (analog->channels().size() > 0) {

            auto chan     = analog->channels()[0];
            auto quantity = QString::fromStdString(analog->mq()->name());
            auto unit     = QString::fromStdString(analog->unit()->name());

            void* data = analog->data_pointer();
            float test = *(float*)data;

            switch (m_Mode) {
                case AquisitionModel::Mode::UNTHROTTLED:
                    self->beginInsertRows({}, m_lRows.size(), m_lRows.size());
                    m_lRows << new Row {
                        m_lRows.size(),
                        0,
                        {
                            new Chan { true, test }
                        },
                        std::chrono::time_point_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now()
                        ).time_since_epoch().count(),
                        unit,
                        quantity,
                        {},
                    };
                    self->endInsertRows();
                    break;
                case AquisitionModel::Mode::MANUAL:
                    m_pLastValue = new Row { //FIXME leak like hell
                        m_lRows.size(),
                        0,
                        {
                            new Chan { true, test }
                        },
                        std::chrono::time_point_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now()
                        ).time_since_epoch().count(),
                        unit,
                        quantity,
                        {},
                    };
            }

            Q_EMIT self->currentValuesChanged({test});
        }
    };

    auto datafeed_callback = [self, analog_datafeed_callback](
        std::shared_ptr<sigrok::Device> dev,
        std::shared_ptr<sigrok::Packet> packet) {

        /* Dispatch into the right callback */
        switch (packet->type()->id()) {
            case SR_DF_HEADER:
                break;
            case SR_DF_END:
                break;
            case SR_DF_META:
                break;
            case SR_DF_TRIGGER:
                break;
            case SR_DF_LOGIC:
                break;
            case SR_DF_ANALOG_OLD:
                break;
            case SR_DF_FRAME_BEGIN:
                break;
            case SR_DF_FRAME_END:
                break;
            case SR_DF_ANALOG:
                auto an = std::dynamic_pointer_cast<sigrok::Analog>(
                    packet->payload()
                );
                analog_datafeed_callback(dev, packet, an);
                break;
        }
    };

    auto stopped_callback = [self]() {
//         qDebug() << "STOPPED";
    };

    m_pSess = m_pContext->create_session();
    m_pSess->add_datafeed_callback(datafeed_callback);
    m_pSess->set_stopped_callback(stopped_callback);
}

AquisitionModel::AquisitionModel(SigrokDevice* dev) :
    QAbstractTableModel(nullptr), d_ptr(new AquisitionModelPrivate())
{
    d_ptr->m_pDev = dev->native();
    d_ptr->m_pContext = DeviceModel::instance()->context();;
}

AquisitionModel::~AquisitionModel()
{
    
}

void AquisitionModel::setMode(Mode m)
{
    d_ptr->m_Mode = m;
}


int AquisitionModel::channelCount() const
{
    return d_ptr->m_lRows.size() ? d_ptr->m_lRows.last()->m_lChans.size() : 0;
}

QList<float> AquisitionModel::currentValues() const
{
    if (!d_ptr->m_lRows.size()) return {};

    QList<float> ret;

    const auto& lastRow = d_ptr->m_lRows.last()->m_lChans;

    for (const auto val : lastRow)
        ret << val->m_AnalogValue;

    return ret;
}

QVariant Row::commonRole(int role) const
{
    switch(role) {
        case (int) AquisitionModel::Role::IS_CHANNEL:
            return false;
        case (int) AquisitionModel::Role::CHANNEL_NAME:
            return QString();
        case (int) AquisitionModel::Role::CHANNEL_ENABLED:
            return QString();
        case (int) AquisitionModel::Role::UNIT:
            return QString();
        case (int) AquisitionModel::Role::UNIT_NAME:
            return QString();
        case (int) AquisitionModel::Role::QUANTITY:
            return QString();
        case (int) AquisitionModel::Role::QUANTITY_NAME:
            return QString();
    };

    return {};
}

QVariant AquisitionModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if (role == Qt::ToolTipRole && !d_ptr->m_lRows[idx.row()]->m_lChans.isEmpty())
        return d_ptr->m_lRows[idx.row()]->m_lChans[0]->m_AnalogValue;

    auto r = d_ptr->m_lRows[idx.row()];

    Q_ASSERT(r);

    switch (idx.column()) {
        case AquisitionModelPrivate::Column::ID:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_Id.isValid() ? r->m_Id :idx.row();
                case (int) AquisitionModel::Role::U_TIMESTAMP:
                    return r->m_Epoch;
            };
            return r->commonRole(role);

        case AquisitionModelPrivate::Column::CHANS:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    if (!r->m_lChans.isEmpty())
                        return r->m_lChans[0]->m_AnalogValue;
                    break;
                case (int) AquisitionModel::Role::IS_CHANNEL:
                    return true;
                case (int) AquisitionModel::Role::CHANNEL_ENABLED:
                    return true;
                case (int) AquisitionModel::Role::UNIT:
                case (int) AquisitionModel::Role::UNIT_NAME:
                    return r->m_Unit;
                case (int) AquisitionModel::Role::QUANTITY:
                case (int) AquisitionModel::Role::QUANTITY_NAME:
                    return r->m_Quantity;
                case (int) AquisitionModel::Role::U_TIMESTAMP:
                    return r->m_Epoch;
            };
    }

    return {};
}

bool AquisitionModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return false;

    if (role == Qt::EditRole && index.column() == 0) {
        auto r = d_ptr->m_lRows[index.row()];

        r->m_Id = value;
        Q_EMIT dataChanged(index, index);

        return true;
    }

    return false;
}

int AquisitionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lRows.size();
}

int AquisitionModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant AquisitionModel::headerData(int sec, Qt::Orientation ori, int role) const
{
    if (ori == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch(sec) {
                case AquisitionModelPrivate::Column::ID:
                    return QStringLiteral("ID");
                case AquisitionModelPrivate::Column::CHANS:
                    return QStringLiteral("Channel 1");
            }
        }
    }

    return {};
}

void AquisitionModel::start()
{
//     if (!(d_ptr->m_pSess && d_ptr->m_pSess->is_running()))
//         return;
//     qDebug() << d_ptr->m_pSess->is_running();

    d_ptr->initialize(this);

    try {
        d_ptr->m_pSess->add_device(d_ptr->m_pDev);
        d_ptr->m_pDev->open();
        d_ptr->m_pSess->start();
    }
    catch (const sigrok::Error& e) {
        qWarning() << "Starting device failed because:" << e.what();
    }

}

void AquisitionModel::stop()
{
    if (!(d_ptr->m_pSess && !d_ptr->m_pSess->is_running()))
        return;

    d_ptr->m_pSess->stop();
    d_ptr->m_pDev->close();
}

void AquisitionModel::clear()
{
    beginResetModel();
    d_ptr->m_lRows.clear(); //TODO leak
    endResetModel();
}

bool AquisitionModel::addLastSample()
{
    if (d_ptr->m_Mode != AquisitionModel::Mode::MANUAL || !d_ptr->m_pLastValue)
        return false;

    if (d_ptr->m_pLastValue
      && (!d_ptr->m_lRows.isEmpty())
      && d_ptr->m_pLastValue == d_ptr->m_lRows.last())
        return false;

    beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
    d_ptr->m_lRows << d_ptr->m_pLastValue;
    endInsertRows();

    return true;
}

