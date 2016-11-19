#include "devicenode.h"

#include "widgets/aquisition.h"
#include "sigrokd/aquisitionmodel.h"

#include <QDebug>
#include <libsigrokcxx/libsigrokcxx.hpp>

#include "sigrokd/devicemodel.h"

class DeviceNodePrivate : public QObject
{
public:
    AquisitionModel* m_pModel {nullptr};
    Aquisition* m_pWidget {nullptr};
    QString m_Title {QStringLiteral("Aquisition")};
    std::shared_ptr<sigrok::HardwareDevice> m_pDevice;

    DeviceNode* q_ptr;

public Q_SLOTS:
    void slotClear();
};

DeviceNode::DeviceNode(QObject* parent) :
    AbstractNode(parent), d_ptr(new DeviceNodePrivate())
{
    d_ptr->q_ptr = this;
}

DeviceNode::~DeviceNode()
{
    
}

QString DeviceNode::title() const
{
    return d_ptr->m_Title;
}

QString DeviceNode::id() const
{
    return QStringLiteral("device_node");
}

void DeviceNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* DeviceNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Aquisition();
    }

    return d_ptr->m_pWidget;
}

void DeviceNode::setModel(AquisitionModel* m)
{
    //TODO disconnect existing
    d_ptr->m_pModel = m;

    if (d_ptr->m_pModel) {
        QObject::connect(d_ptr->m_pWidget, &Aquisition::started, d_ptr->m_pModel,
            &AquisitionModel::start);
        QObject::connect(d_ptr->m_pWidget, &Aquisition::stopped, d_ptr->m_pModel,
            &AquisitionModel::stop);
        QObject::connect(d_ptr->m_pWidget, &Aquisition::cleared, d_ptr,
            &DeviceNodePrivate::slotClear);
    }
}

QAbstractItemModel* DeviceNode::model() const
{
    return d_ptr->m_pModel;
}

std::shared_ptr<sigrok::HardwareDevice> DeviceNode::device() const
{
    return d_ptr->m_pDevice;
}

void DeviceNode::setDevice(std::shared_ptr<sigrok::HardwareDevice> dev)
{
    if (d_ptr->m_pDevice == dev)
        return;

    //TODO if device and running, ask to stop

    d_ptr->m_pDevice = dev;

    d_ptr->m_Title = QString::fromStdString(dev->model());

    Q_EMIT titleChanged(d_ptr->m_Title);

    auto devm = DeviceModel::instance();

    auto ctx = devm->context();

    setModel(new AquisitionModel(ctx, dev));
}

bool DeviceNode::dummy() const{
    // do nothing, the property is a trigger
    return false;
}

void DeviceNodePrivate::slotClear()
{
    Q_EMIT q_ptr->aboutToClear();
    m_pModel->clear();
    Q_EMIT q_ptr->cleared();
}

void DeviceNode::clear(bool value)
{
    d_ptr->slotClear();
}

void DeviceNode::start(bool value)
{
    d_ptr->m_pModel->start();
}

void DeviceNode::stop(bool value)
{
    d_ptr->m_pModel->stop();
}
