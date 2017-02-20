#include "liveacquisitionnode.h"

#include "widgets/acquisition.h"
#include "sigrokd/acquisitionmodel.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "sigrokd/devicemodel.h"

#include "sigrokd/sigrokdevice.h"

#ifndef DECL_QAbstractItemModel
#define DECL_QAbstractItemModel
Q_DECLARE_STREAM_METATYPE(SigrokDevice*)
#endif

class AcquisitionNodePrivate : public QObject
{
    Q_OBJECT
public:
    AcquisitionModel* m_pModel {nullptr};
    Acquisition* m_pWidget {nullptr};
    QString m_Title {QStringLiteral("Live Acquisition")};
    SigrokDevice* m_pDevice;
    QTimer* m_pTimer {nullptr};

    AcquisitionNode* q_ptr;

public Q_SLOTS:
    void slotClear();
    void slotStop();
    void slotStart();
    void slotState(AcquisitionModel::State, AcquisitionModel::State);
    void slotTick();
    void slotDestroyed();
};

AcquisitionNode::AcquisitionNode(AbstractSession* sess) :
    AbstractNode(sess), d_ptr(new AcquisitionNodePrivate())
{
    d_ptr->q_ptr = this;
}

AcquisitionNode::~AcquisitionNode()
{
    if (isRunning())
        d_ptr->m_pModel->stop();
    delete d_ptr;
}

void AcquisitionNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* AcquisitionNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Acquisition(false);

        QObject::connect(d_ptr->m_pWidget, &Acquisition::started, d_ptr,
            &AcquisitionNodePrivate::slotStart);
        QObject::connect(d_ptr->m_pWidget, &Acquisition::stopped, d_ptr,
            &AcquisitionNodePrivate::slotStop);
        QObject::connect(d_ptr->m_pWidget, &Acquisition::cleared, d_ptr,
            &AcquisitionNodePrivate::slotClear);
        QObject::connect(d_ptr->m_pWidget, &QObject::destroyed, d_ptr,
            &AcquisitionNodePrivate::slotDestroyed);
    }

    return d_ptr->m_pWidget;
}

bool AcquisitionNode::isRunning() const
{
    if (!d_ptr->m_pModel)
        return false;

    return d_ptr->m_pModel->state() == AcquisitionModel::State::STARTED;
}

void AcquisitionNode::setModel(AcquisitionModel* m)
{
    if (m == d_ptr->m_pModel)
        return;

    if (d_ptr->m_pModel)
        disconnect(d_ptr->m_pModel, &AcquisitionModel::stateChanged,
            d_ptr, &AcquisitionNodePrivate::slotState);

    d_ptr->m_pModel = m;

    connect(d_ptr->m_pModel, &AcquisitionModel::stateChanged,
        d_ptr, &AcquisitionNodePrivate::slotState);

    Q_EMIT modelChanged(m);
}

AcquisitionModel* AcquisitionNode::aqModel() const
{
    return d_ptr->m_pModel;
}

QAbstractItemModel* AcquisitionNode::model() const
{
    return d_ptr->m_pModel;
}

SigrokDevice* AcquisitionNode::device() const
{
    return d_ptr->m_pDevice;
}

void AcquisitionNode::setDevice(SigrokDevice* dev)
{
    if (d_ptr->m_pDevice == dev)
        return;

    //TODO if device and running, ask to stop

    d_ptr->m_pDevice = dev;

    d_ptr->m_Title = dev->name();

    Q_EMIT titleChanged(d_ptr->m_Title);

    DeviceModel::instance();

    setModel(new AcquisitionModel(dev));
}

bool AcquisitionNode::dummy() const{
    // do nothing, the property is a trigger
    return false;
}

void AcquisitionNodePrivate::slotClear()
{
    if (!m_pModel)
        return;

    Q_EMIT q_ptr->aboutToClear();
    m_pModel->clear();
    Q_EMIT q_ptr->cleared();
}

void AcquisitionNodePrivate::slotStop()
{
    if (!m_pModel)
        return;

    if (m_pModel->state() == AcquisitionModel::State::STOPPED)
        return;

    m_pModel->stop();

}

void AcquisitionNodePrivate::slotStart()
{
    if (!m_pModel)
        return;

    switch(m_pModel->state()) {
        case AcquisitionModel::State::STOPPED:
        case AcquisitionModel::State::TIMEOUT:
        case AcquisitionModel::State::ERROR:
            break;
        case AcquisitionModel::State::STARTED:
        case AcquisitionModel::State::INIT:
        case AcquisitionModel::State::IDLE:
            return;
    }

    m_pModel->start();

    Q_EMIT q_ptr->runningChanged(true);
}

void AcquisitionNodePrivate::slotState(AcquisitionModel::State n, AcquisitionModel::State o)
{
    Q_UNUSED(o)

    static const QString stopped = tr("Acquisition stopped");
    static const QString started = tr("Acquisition in progress");
    static const QString error   = tr("Acquisition failed");
    static const QString timeout = tr("Sample acquisition timed out");
    static const QString idle    = tr("Idle");
    static const QString init    = tr("Initializing");
    switch(n) {
        case AcquisitionModel::State::ERROR:
        case AcquisitionModel::State::STOPPED:
            if (m_pTimer) {
                m_pTimer->stop();
            }
            break;
        case AcquisitionModel::State::TIMEOUT:
        case AcquisitionModel::State::STARTED:
            if (!m_pTimer) {
                m_pTimer = new QTimer(this);
                connect(m_pTimer, &QTimer::timeout, this, &AcquisitionNodePrivate::slotTick);
            }

            if (!m_pTimer->isActive()) {
                m_pTimer->setInterval(1000);
                m_pTimer->start();
            }
            break;
        case AcquisitionModel::State::INIT:
        case AcquisitionModel::State::IDLE:
            break;
    }

    switch(n) {
        case AcquisitionModel::State::STOPPED:
            if (m_pWidget)
                m_pWidget->setStatus(stopped);
            Q_EMIT q_ptr->notify(stopped);
            Q_EMIT q_ptr->runningChanged(false);
            break;
        case AcquisitionModel::State::TIMEOUT:
            if (m_pWidget)
                m_pWidget->setStatus(timeout);
            Q_EMIT q_ptr->notify(timeout);
            Q_EMIT q_ptr->runningChanged(false);
            break;
        case AcquisitionModel::State::STARTED:
            if (m_pWidget)
                m_pWidget->setStatus(started);
            Q_EMIT q_ptr->notify(started);
            Q_EMIT q_ptr->runningChanged(true);
            break;
        case AcquisitionModel::State::ERROR:
            if (m_pWidget)
                m_pWidget->setStatus(error);
            Q_EMIT q_ptr->notify(error);
            Q_EMIT q_ptr->runningChanged(false);
            break;
        case AcquisitionModel::State::INIT:
            if (m_pWidget)
                m_pWidget->setStatus(init);
            break;
        case AcquisitionModel::State::IDLE:
            if (m_pWidget)
                m_pWidget->setStatus(idle);
            break;
    }
}

void AcquisitionNodePrivate::slotTick()
{
    if (m_pModel && m_pWidget) {
        m_pWidget->setCount(m_pModel->rowCount());
        m_pWidget->setLast(m_pModel->lastSampleDateTime());
    }
}

void AcquisitionNodePrivate::slotDestroyed()
{
    m_pWidget = nullptr;
}

void AcquisitionNode::clear(bool value)
{
    Q_UNUSED(value)
    d_ptr->slotClear();
}

void AcquisitionNode::start(bool value)
{
    Q_UNUSED(value)

    d_ptr->slotStart();
}

void AcquisitionNode::stop(bool value)
{
    Q_UNUSED(value)

    d_ptr->slotStop();
}

#include <liveacquisitionnode.moc>
