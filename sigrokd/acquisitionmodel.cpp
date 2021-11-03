#include "acquisitionmodel.h"

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
    int m_UnitId;
    QString m_Unit;
    int m_QuantityId;
    QString m_Quantity;
    QVariant m_Id;

    //Helpers
    QVariant commonRole(int role) const;
};

class AcquisitionModelPrivate
{
public:
    enum Column {
        ID,
        CHANS
    };

    enum class Action {
        NEW_SAMPLE,
        MISSED,
        ERROR,
        STOPPING,
        STARTING,
    };
    Q_ENUMS(AcquisitionModelPrivate::Action)

    QVector<Row*> m_lRows;
    Row* m_pLastValue {nullptr};
    AcquisitionModel::State m_State {AcquisitionModel::State::STOPPED};
    AcquisitionModel::Mode m_Mode {AcquisitionModel::Mode::UNTHROTTLED};
    std::shared_ptr<sigrok::HardwareDevice> m_pDev;
    std::shared_ptr<sigrok::Session> m_pSess;
    std::shared_ptr<sigrok::Context> m_pContext;

    void initialize(AcquisitionModel* self);

    void performAction(Action a);

    AcquisitionModel* q_ptr;
};

void AcquisitionModelPrivate::initialize(AcquisitionModel* self)
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


            const sigrok::Quantity* quantity = nullptr;

            try {
                quantity = analog->mq();
            }
            catch(const sigrok::Error& e) {}

            auto unit     = analog->unit();

            void* data = analog->data_pointer();
            float test = *(float*)data;

            performAction(Action::NEW_SAMPLE);

            switch (m_Mode) {
                case AcquisitionModel::Mode::UNTHROTTLED:
                    self->beginInsertRows({}, m_lRows.size(), m_lRows.size());
                    m_lRows << new Row {
                        m_lRows.size(),
                        0,
                        QVector<Chan*> {
                            new Chan { true, test }
                        },
                        std::chrono::time_point_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now()
                        ).time_since_epoch().count(),
                        unit->id(),
                        QString::fromStdString(unit->name()),
                        quantity ? quantity->id() : 0,
                        quantity ? QString::fromStdString(quantity->name()) : QString(),
                        {},
                    };
                    self->endInsertRows();
                    break;
                case AcquisitionModel::Mode::MANUAL:
                    m_pLastValue = new Row { //FIXME leak like hell
                        m_lRows.size(),
                        0,
                        {
                            new Chan { true, test }
                        },
                        std::chrono::time_point_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now()
                        ).time_since_epoch().count(),
                        unit->id(),
                        QString::fromStdString(unit->name()),
                        quantity ? quantity->id() : 0,
                        quantity ? QString::fromStdString(quantity->name()) : QString(),
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

AcquisitionModel::AcquisitionModel(SigrokDevice* dev) :
    QAbstractTableModel(nullptr), d_ptr(new AcquisitionModelPrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_pDev = dev->native();
    d_ptr->m_pContext = DeviceModel::instance()->context();;
}

AcquisitionModel::~AcquisitionModel()
{
    delete d_ptr;
}

void AcquisitionModel::setMode(Mode m)
{
    d_ptr->m_Mode = m;
}


int AcquisitionModel::channelCount() const
{
    return d_ptr->m_lRows.size() ? d_ptr->m_lRows.last()->m_lChans.size() : 0;
}

QList<float> AcquisitionModel::currentValues() const
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
        case (int) AcquisitionModel::Role::IS_CHANNEL:
            return false;
        case (int) AcquisitionModel::Role::CHANNEL_NAME:
            return QString();
        case (int) AcquisitionModel::Role::CHANNEL_ENABLED:
            return QString();
        case (int) AcquisitionModel::Role::UNIT:
            return QString();
        case (int) AcquisitionModel::Role::UNIT_NAME:
            return QString();
        case (int) AcquisitionModel::Role::QUANTITY:
            return QString();
        case (int) AcquisitionModel::Role::QUANTITY_NAME:
            return QString();
    };

    return {};
}

QVariant AcquisitionModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    if (role == Qt::ToolTipRole && !d_ptr->m_lRows[idx.row()]->m_lChans.isEmpty())
        return d_ptr->m_lRows[idx.row()]->m_lChans[0]->m_AnalogValue;

    auto r = d_ptr->m_lRows[idx.row()];

    Q_ASSERT(r);

    switch (idx.column()) {
        case AcquisitionModelPrivate::Column::ID:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_Id.isValid() ? r->m_Id :idx.row();
                case (int) AcquisitionModel::Role::U_TIMESTAMP:
                    return r->m_Epoch;
            };
            return r->commonRole(role);

        case AcquisitionModelPrivate::Column::CHANS:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    if (!r->m_lChans.isEmpty())
                        return r->m_lChans[0]->m_AnalogValue;
                    break;
                case (int) AcquisitionModel::Role::IS_CHANNEL:
                    return true;
                case (int) AcquisitionModel::Role::CHANNEL_ENABLED:
                    return true;
                case (int) AcquisitionModel::Role::UNIT:
                    return r->m_UnitId;
                case (int) AcquisitionModel::Role::UNIT_NAME:
                    return r->m_Unit;
                case (int) AcquisitionModel::Role::QUANTITY:
                case (int) AcquisitionModel::Role::QUANTITY_NAME:
                    return r->m_Quantity;
                case (int) AcquisitionModel::Role::U_TIMESTAMP:
                    return r->m_Epoch;
            };
    }

    return {};
}

bool AcquisitionModel::setData(const QModelIndex &index, const QVariant &value, int role)
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

int AcquisitionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lRows.size();
}

int AcquisitionModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 2;
}

QVariant AcquisitionModel::headerData(int sec, Qt::Orientation ori, int role) const
{
    if (ori == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch(sec) {
                case AcquisitionModelPrivate::Column::ID:
                    return QStringLiteral("ID");
                case AcquisitionModelPrivate::Column::CHANS:
                    return QStringLiteral("Channel 1");
            }
        }
    }

    return {};
}

void AcquisitionModel::start()
{
//     if (!(d_ptr->m_pSess && d_ptr->m_pSess->is_running()))
//         return;
//     qDebug() << d_ptr->m_pSess->is_running();

    d_ptr->initialize(this);

    try {
        d_ptr->m_pSess->add_device(d_ptr->m_pDev);
        d_ptr->m_pDev->open();
        d_ptr->m_pSess->start();
        d_ptr->performAction(AcquisitionModelPrivate::Action::STARTING);
    }
    catch (const sigrok::Error& e) {
        qWarning() << "Starting device failed because:" << e.what();
    }

}

void AcquisitionModel::stop()
{
    if (!(d_ptr->m_pSess && d_ptr->m_pSess->is_running()))
        return;

    d_ptr->performAction(AcquisitionModelPrivate::Action::STOPPING);

    d_ptr->m_pSess->stop();
    d_ptr->m_pDev->close();
}

void AcquisitionModel::clear()
{
    beginResetModel();
    d_ptr->m_lRows.clear(); //TODO leak
    endResetModel();
}

bool AcquisitionModel::addLastSample()
{
    if (d_ptr->m_Mode != AcquisitionModel::Mode::MANUAL || !d_ptr->m_pLastValue)
        return false;

    if (d_ptr->m_pLastValue
      && (!d_ptr->m_lRows.isEmpty())
      && d_ptr->m_pLastValue == d_ptr->m_lRows.last()) {
        d_ptr->performAction(AcquisitionModelPrivate::Action::MISSED);
        return false;
    }

    beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
    d_ptr->m_lRows << d_ptr->m_pLastValue;
    endInsertRows();

    return true;
}

void AcquisitionModelPrivate::performAction(Action a)
{
    typedef AcquisitionModel::State S;

    // The idea is to have a deterministic state
    constexpr static const AcquisitionModel::State matrix[6][5] = {
        /*              NEW_SAMPLE   MISSED      ERROR     STOPPING  STARTING */
        /*STOPPED */ { S::ERROR   , S::ERROR  , S::ERROR , S::STOPPED, S::INIT },
        /*INIT    */ { S::STARTED , S::ERROR  , S::ERROR , S::STOPPED, S::ERROR},
        /*IDLE    */ { S::STARTED , S::TIMEOUT, S::ERROR , S::STOPPED, S::ERROR},
        /*TIMEOUT */ { S::STARTED , S::TIMEOUT, S::ERROR , S::STOPPED, S::ERROR},
        /*STARTED */ { S::STARTED , S::TIMEOUT, S::ERROR , S::STOPPED, S::ERROR},
        /*ERROR   */ { S::ERROR   , S::ERROR  , S::ERROR , S::STOPPED, S::ERROR}
    };

    const auto old = m_State;
    m_State = matrix[(int)m_State][(int)a];

    if (old != m_State) {
        Q_EMIT q_ptr->stateChanged(m_State, old);
    }
}

AcquisitionModel::State AcquisitionModel::state() const
{
    return d_ptr->m_State;
}

QDateTime AcquisitionModel::lastSampleDateTime() const
{
    if (d_ptr->m_lRows.isEmpty())
        return {};

    return QDateTime::fromMSecsSinceEpoch(d_ptr->m_lRows.last()->m_Epoch);
}

