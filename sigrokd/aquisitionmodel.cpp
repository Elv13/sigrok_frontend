#include "aquisitionmodel.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>

#include <libsigrokcxx/libsigrokcxx.hpp>

struct Chan {
    bool  m_IsEnabled;
    float m_AnalogValue;
};

struct Row {
    int m_Index;
    unsigned long long m_TimeStamp;
    QVector<Chan*> m_lChans;
    QDateTime m_Time;
    QString m_Unit;
    QString m_Quantity;

    //Helpers
    QVariant commonRole(int role) const;
};

class AquisitionModelPrivate
{
public:
    enum Column {
        ID,
        DATE,
        TIMESTAMP,
        UNIT,
        QUANTITY,
        CHANS
    };

    QVector<Row*> m_lRows;
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
        if (analog->channels().size() > 0) {

            auto chan     = analog->channels()[0];
            auto quantity = QString::fromStdString(analog->mq()->name());
            auto unit     = QString::fromStdString(analog->unit()->name());

            void* data = analog->data_pointer();
            float test = *(float*)data;

            self->beginInsertRows({}, m_lRows.size(), m_lRows.size());
            m_lRows << new Row {
                m_lRows.size(),
                0,
                {
                    new Chan { true, test }
                },
                {},
                unit,
                quantity,
            };
            self->endInsertRows();

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

AquisitionModel::AquisitionModel(std::shared_ptr<sigrok::Context> ctx, std::shared_ptr<sigrok::HardwareDevice> dev) :
    QAbstractTableModel(nullptr), d_ptr(new AquisitionModelPrivate())
{
    d_ptr->m_pDev = dev;
    d_ptr->m_pContext = ctx;
}

AquisitionModel::~AquisitionModel()
{
    
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

    if (role == Qt::ToolTipRole)
        return d_ptr->m_lRows[idx.row()]->m_lChans[0]->m_AnalogValue;

    auto r = d_ptr->m_lRows[idx.row()];

    switch (idx.column()) {
        case AquisitionModelPrivate::Column::ID:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return idx.row();
            };
            return r->commonRole(role);

        case AquisitionModelPrivate::Column::DATE:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_Time;
            };
            return r->commonRole(role);

        case AquisitionModelPrivate::Column::TIMESTAMP:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_TimeStamp;
            };
            return r->commonRole(role);

        case AquisitionModelPrivate::Column::UNIT:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_Unit;
            };
            return r->commonRole(role);

        case AquisitionModelPrivate::Column::QUANTITY:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_Quantity;
            };
            return r->commonRole(role);

        case AquisitionModelPrivate::Column::CHANS:

            switch(role) {
                case Qt::DisplayRole:
                case Qt::EditRole:
                    return r->m_lChans[0]->m_AnalogValue;
                case (int) AquisitionModel::Role::IS_CHANNEL:
                    return true;
            };
            return r->commonRole(role);
    }

    return {};
}

int AquisitionModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lRows.size();
}

int AquisitionModel::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 6;
}

QVariant AquisitionModel::headerData(int sec, Qt::Orientation ori, int role) const
{
    if (ori == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            switch(sec) {
                case AquisitionModelPrivate::Column::ID:
                    return QStringLiteral("ID");
                case AquisitionModelPrivate::Column::DATE:
                    return QStringLiteral("Date");
                case AquisitionModelPrivate::Column::TIMESTAMP:
                    return QStringLiteral("TimeStamp");
                case AquisitionModelPrivate::Column::UNIT:
                    return QStringLiteral("Unit");
                case AquisitionModelPrivate::Column::QUANTITY:
                    return QStringLiteral("Quantity");
                case AquisitionModelPrivate::Column::CHANS:
                    return QStringLiteral("Channel 1");
            }
        }
    }

    return {};
}

void AquisitionModel::start()
{
    d_ptr->initialize(this);

    try {
        d_ptr->m_pSess->add_device(d_ptr->m_pDev);
    }
    catch (const sigrok::Error& e) {
        qWarning() << "Starting device failed because:" << e.what();
    }

    d_ptr->m_pDev->open();
    d_ptr->m_pSess->start();
}

void AquisitionModel::stop()
{
    d_ptr->m_pSess->stop();
    d_ptr->m_pDev->close();
}

void AquisitionModel::clear()
{
    beginResetModel();
    d_ptr->m_lRows.clear(); //TODO leak
    endResetModel();
}

