#include "devicenode.h"

#include "../widgets/aquisition.h"
#include "../aquisitionmodel.h"

#include <QDebug>
#include <libsigrokcxx/libsigrokcxx.hpp>

#include "devicemodel.h"

class DeviceNodePrivate
{
public:
    AquisitionModel* m_pModel {nullptr};
    Aquisition* m_pWidget {nullptr};
    QString m_Title {QStringLiteral("Aquisition")};
    std::shared_ptr<sigrok::HardwareDevice> m_pDevice;
};

DeviceNode::DeviceNode(QObject* parent) :
    AbstractNode(parent), d_ptr(new DeviceNodePrivate())
{
//     d_ptr->m_Title = name;


}

// DeviceNode::DeviceNode(QObject* parent) :
//     AbstractNode(parent), d_ptr(new DeviceNodePrivate())
// {
//     auto devm = DeviceModel::instance();
// 
//     auto ctx = devm->context();
//     auto dev = devm->currentDevice();
// 
//     setModel(new AquisitionModel(ctx, dev));
// }

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
        QObject::connect(d_ptr->m_pWidget->m_pStart, &QPushButton::clicked, d_ptr->m_pModel,
            &AquisitionModel::start);
        QObject::connect(d_ptr->m_pWidget->m_pStop, &QPushButton::clicked, d_ptr->m_pModel,
            &AquisitionModel::stop);
    }
}

QAbstractItemModel* DeviceNode::model() const
{
    qDebug() << "IN MODEL GETTER" << d_ptr->m_pModel;
    return d_ptr->m_pModel;
}

std::shared_ptr<sigrok::HardwareDevice> DeviceNode::device() const
{
    return d_ptr->m_pDevice;
}

void DeviceNode::setDevice(std::shared_ptr<sigrok::HardwareDevice> dev)
{
    //TODO if device and running, ask to stop

    d_ptr->m_pDevice = dev;

    d_ptr->m_Title = QString::fromStdString(dev->model());

    Q_EMIT titleChanged(d_ptr->m_Title);

    qDebug() << "\n\n\nSET DEVICE" << d_ptr->m_Title <<QString::fromStdString(dev->vendor());

    auto devm = DeviceModel::instance();

    auto ctx = devm->context();

    setModel(new AquisitionModel(ctx, dev));
}
