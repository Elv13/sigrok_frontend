#include "aquisitionnode.h"

#include "widgets/aquisition.h"
#include "sigrokd/aquisitionmodel.h"

#include <QtCore/QDebug>

#include "sigrokd/devicemodel.h"

#include "sigrokd/sigrokdevice.h"

#ifndef DECL_QAbstractItemModel
#define DECL_QAbstractItemModel
Q_DECLARE_STREAM_METATYPE(SigrokDevice*)
#endif

class AquisitionNodePrivate : public QObject
{
public:
    AquisitionModel* m_pModel {nullptr};
    Aquisition* m_pWidget {nullptr};
    QString m_Title {QStringLiteral("Live Aquisition")};
    SigrokDevice* m_pDevice;
    bool m_IsRunning {false};

    AquisitionNode* q_ptr;

public Q_SLOTS:
    void slotClear();
    void slotStop();
    void slotStart();
};

AquisitionNode::AquisitionNode(QObject* parent) :
    AbstractNode(parent), d_ptr(new AquisitionNodePrivate())
{
    d_ptr->q_ptr = this;
}

AquisitionNode::~AquisitionNode()
{
    
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
    }

    return d_ptr->m_pWidget;
}

bool AquisitionNode::isRunning() const
{
    return d_ptr->m_IsRunning;
}

void AquisitionNode::setModel(AquisitionModel* m)
{
    if (m == d_ptr->m_pModel)
        return;

    d_ptr->m_pModel = m;
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

    if (!m_IsRunning)
        return;

    m_pModel->stop();

    m_IsRunning = false;

    Q_EMIT q_ptr->runningChanged(m_IsRunning);
}

void AquisitionNodePrivate::slotStart()
{
    qDebug() << "IN START";
    if (!m_pModel)
        return;

    if (m_IsRunning)
        return;

    qDebug() << "IN START2";
    m_pModel->start();

    m_IsRunning = true;

    Q_EMIT q_ptr->runningChanged(m_IsRunning);
    qDebug() << "IN START3";
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
