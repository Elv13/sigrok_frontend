#include "aquisitionnode.h"

#include "widgets/aquisition.h"
#include "sigrokd/aquisitionmodel.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "sigrokd/devicemodel.h"

#include "sigrokd/sigrokdevice.h"

#ifndef DECL_QAbstractItemModel
#define DECL_QAbstractItemModel
Q_DECLARE_STREAM_METATYPE(SigrokDevice*)
#endif

class AquisitionNodePrivate : public QObject
{
    Q_OBJECT
public:
    AquisitionModel* m_pModel {nullptr};
    Aquisition* m_pWidget {nullptr};
    QString m_Title {QStringLiteral("Live Aquisition")};
    SigrokDevice* m_pDevice;
    QTimer* m_pTimer {nullptr};

    AquisitionNode* q_ptr;

public Q_SLOTS:
    void slotClear();
    void slotStop();
    void slotStart();
    void slotState(AquisitionModel::State, AquisitionModel::State);
    void slotTick();
    void slotDestroyed();
};

AquisitionNode::AquisitionNode(AbstractSession* sess) :
    AbstractNode(sess), d_ptr(new AquisitionNodePrivate())
{
    d_ptr->q_ptr = this;
}

AquisitionNode::~AquisitionNode()
{
    if (isRunning())
        d_ptr->m_pModel->stop();
    delete d_ptr;
}

QString AquisitionNode::title() const
{
    return d_ptr->m_Title;
}

QString AquisitionNode::id() const
{
    return QStringLiteral("aquisition_node");
}

void AquisitionNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* AquisitionNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Aquisition(false);

        QObject::connect(d_ptr->m_pWidget, &Aquisition::started, d_ptr,
            &AquisitionNodePrivate::slotStart);
        QObject::connect(d_ptr->m_pWidget, &Aquisition::stopped, d_ptr,
            &AquisitionNodePrivate::slotStop);
        QObject::connect(d_ptr->m_pWidget, &Aquisition::cleared, d_ptr,
            &AquisitionNodePrivate::slotClear);
        QObject::connect(d_ptr->m_pWidget, &QObject::destroyed, d_ptr,
            &AquisitionNodePrivate::slotDestroyed);
    }

    return d_ptr->m_pWidget;
}

bool AquisitionNode::isRunning() const
{
    if (!d_ptr->m_pModel)
        return false;

    return d_ptr->m_pModel->state() == AquisitionModel::State::STARTED;
}

void AquisitionNode::setModel(AquisitionModel* m)
{
    if (m == d_ptr->m_pModel)
        return;

    if (d_ptr->m_pModel)
        disconnect(d_ptr->m_pModel, &AquisitionModel::stateChanged,
            d_ptr, &AquisitionNodePrivate::slotState);

    d_ptr->m_pModel = m;

    connect(d_ptr->m_pModel, &AquisitionModel::stateChanged,
        d_ptr, &AquisitionNodePrivate::slotState);

    Q_EMIT modelChanged(m);
}

AquisitionModel* AquisitionNode::aqModel() const
{
    return d_ptr->m_pModel;
}

QAbstractItemModel* AquisitionNode::model() const
{
    return d_ptr->m_pModel;
}

SigrokDevice* AquisitionNode::device() const
{
    return d_ptr->m_pDevice;
}

void AquisitionNode::setDevice(SigrokDevice* dev)
{
    if (d_ptr->m_pDevice == dev)
        return;

    //TODO if device and running, ask to stop

    d_ptr->m_pDevice = dev;

    d_ptr->m_Title = dev->name();

    Q_EMIT titleChanged(d_ptr->m_Title);

    DeviceModel::instance();

    setModel(new AquisitionModel(dev));
}

bool AquisitionNode::dummy() const{
    // do nothing, the property is a trigger
    return false;
}

void AquisitionNodePrivate::slotClear()
{
    if (!m_pModel)
        return;

    Q_EMIT q_ptr->aboutToClear();
    m_pModel->clear();
    Q_EMIT q_ptr->cleared();
}

void AquisitionNodePrivate::slotStop()
{
    if (!m_pModel)
        return;

    if (m_pModel->state() == AquisitionModel::State::STOPPED)
        return;

    m_pModel->stop();

}

void AquisitionNodePrivate::slotStart()
{
    if (!m_pModel)
        return;

    switch(m_pModel->state()) {
        case AquisitionModel::State::STOPPED:
        case AquisitionModel::State::TIMEOUT:
        case AquisitionModel::State::ERROR:
            break;
        case AquisitionModel::State::STARTED:
        case AquisitionModel::State::INIT:
        case AquisitionModel::State::IDLE:
            return;
    }

    m_pModel->start();

    Q_EMIT q_ptr->runningChanged(true);
}

void AquisitionNodePrivate::slotState(AquisitionModel::State n, AquisitionModel::State o)
{
    Q_UNUSED(o)

    static const QString stopped = tr("Acquisition stopped");
    static const QString started = tr("Acquisition in progress");
    static const QString error   = tr("Acquisition failed");
    static const QString timeout = tr("Sample acquisition timed out");
    static const QString idle    = tr("Idle");
    static const QString init    = tr("Initializing");
    switch(n) {
        case AquisitionModel::State::ERROR:
        case AquisitionModel::State::STOPPED:
            if (m_pTimer) {
                m_pTimer->stop();
            }
            break;
        case AquisitionModel::State::TIMEOUT:
        case AquisitionModel::State::STARTED:
            if (!m_pTimer) {
                m_pTimer = new QTimer(this);
                connect(m_pTimer, &QTimer::timeout, this, &AquisitionNodePrivate::slotTick);
            }

            if (!m_pTimer->isActive()) {
                m_pTimer->setInterval(1000);
                m_pTimer->start();
            }
            break;
        case AquisitionModel::State::INIT:
        case AquisitionModel::State::IDLE:
            break;
    }

    switch(n) {
        case AquisitionModel::State::STOPPED:
            if (m_pWidget)
                m_pWidget->setStatus(stopped);
            Q_EMIT q_ptr->notify(stopped);
            Q_EMIT q_ptr->runningChanged(false);
            break;
        case AquisitionModel::State::TIMEOUT:
            if (m_pWidget)
                m_pWidget->setStatus(timeout);
            Q_EMIT q_ptr->notify(timeout);
            Q_EMIT q_ptr->runningChanged(false);
            break;
        case AquisitionModel::State::STARTED:
            if (m_pWidget)
                m_pWidget->setStatus(started);
            Q_EMIT q_ptr->notify(started);
            Q_EMIT q_ptr->runningChanged(true);
            break;
        case AquisitionModel::State::ERROR:
            if (m_pWidget)
                m_pWidget->setStatus(error);
            Q_EMIT q_ptr->notify(error);
            Q_EMIT q_ptr->runningChanged(false);
            break;
        case AquisitionModel::State::INIT:
            if (m_pWidget)
                m_pWidget->setStatus(init);
            break;
        case AquisitionModel::State::IDLE:
            if (m_pWidget)
                m_pWidget->setStatus(idle);
            break;
    }
}

void AquisitionNodePrivate::slotTick()
{
    if (m_pModel && m_pWidget) {
        m_pWidget->setCount(m_pModel->rowCount());
        m_pWidget->setLast(m_pModel->lastSampleDateTime());
    }
}

void AquisitionNodePrivate::slotDestroyed()
{
    m_pWidget = nullptr;
}

void AquisitionNode::clear(bool value)
{
    Q_UNUSED(value)
    d_ptr->slotClear();
}

void AquisitionNode::start(bool value)
{
    Q_UNUSED(value)

    d_ptr->slotStart();
}

void AquisitionNode::stop(bool value)
{
    Q_UNUSED(value)

    d_ptr->slotStop();
}

#include <aquisitionnode.moc>
