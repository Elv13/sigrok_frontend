#include "devicenode.h"

#include "../widgets/aquisition.h"
#include "../aquisitionmodel.h"

#include <QDebug>

#include "devicemodel.h"

class DeviceNodePrivate
{
public:
    AquisitionModel* m_pModel {nullptr};
    Aquisition* m_pWidget {nullptr};
    QString m_Title;
};

DeviceNode::DeviceNode(const QString &name, std::shared_ptr<sigrok::HardwareDevice> dev, QObject* parent) :
    AbstractNode(parent), d_ptr(new DeviceNodePrivate())
{
    d_ptr->m_Title = name;

    auto devm = DeviceModel::instance();

    auto ctx = devm->context();

    setModel(new AquisitionModel(ctx, dev));
}

DeviceNode::DeviceNode(QObject* parent) :
    AbstractNode(parent), d_ptr(new DeviceNodePrivate())
{
    auto devm = DeviceModel::instance();

    auto ctx = devm->context();
    auto dev = devm->currentDevice();

    setModel(new AquisitionModel(ctx, dev));
}

DeviceNode::~DeviceNode()
{
    
}

QString DeviceNode::title() const
{
    return d_ptr->m_Title;
}

QWidget* DeviceNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Aquisition();

        if (d_ptr->m_pModel) {
            QObject::connect(d_ptr->m_pWidget->m_pStart, &QPushButton::clicked, d_ptr->m_pModel,
                &AquisitionModel::start);
            QObject::connect(d_ptr->m_pWidget->m_pStop, &QPushButton::clicked, d_ptr->m_pModel,
                &AquisitionModel::stop);
        }
    }

    return d_ptr->m_pWidget;
}

void DeviceNode::setModel(AquisitionModel* m)
{
    //TODO disconnect existing
    d_ptr->m_pModel = m;
}

QAbstractItemModel* DeviceNode::model() const
{
    qDebug() << "IN MODEL GETTER" << d_ptr->m_pModel;
    return d_ptr->m_pModel;
}
