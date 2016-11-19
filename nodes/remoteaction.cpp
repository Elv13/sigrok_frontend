#include "remoteaction.h"

#include "widgets/aquisition.h"
#include "aquisitionmodel.h"

#include <QDebug>
#include <libsigrokcxx/libsigrokcxx.hpp>

#include "models/remotewidgets.h"

class RemoteActionNodePrivate : public QObject
{
public:
    AquisitionModel* m_pModel {nullptr};
    Aquisition* m_pWidget {nullptr};
    QString m_Title {QStringLiteral("Aquisition")};
    std::shared_ptr<sigrok::HardwareDevice> m_pDevice;

    RemoteActionNode* q_ptr;

public Q_SLOTS:
    void slotClear();
};

RemoteActionNode::RemoteActionNode(QObject* parent) :
    AbstractNode(parent), d_ptr(new RemoteActionNodePrivate())
{
//     d_ptr->m_Title = name;
    d_ptr->q_ptr = this;

}

// RemoteActionNode::RemoteActionNode(QObject* parent) :
//     AbstractNode(parent), d_ptr(new RemoteActionNodePrivate())
// {
//     auto devm = DeviceModel::instance();
// 
//     auto ctx = devm->context();
//     auto dev = devm->currentDevice();
// 
//     setModel(new AquisitionModel(ctx, dev));
// }

RemoteActionNode::~RemoteActionNode()
{
    
}

QString RemoteActionNode::title() const
{
    return d_ptr->m_Title;
}

QString RemoteActionNode::id() const
{
    return QStringLiteral("device_node");
}

void RemoteActionNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QWidget* RemoteActionNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Aquisition();
    }

    return d_ptr->m_pWidget;
}

void RemoteActionNode::setModel(AquisitionModel* m)
{
    //TODO disconnect existing
    d_ptr->m_pModel = m;

    if (d_ptr->m_pModel) {
        QObject::connect(d_ptr->m_pWidget->m_pStart, &QPushButton::clicked, d_ptr->m_pModel,
            &AquisitionModel::start);
        QObject::connect(d_ptr->m_pWidget->m_pStop, &QPushButton::clicked, d_ptr->m_pModel,
            &AquisitionModel::stop);
        QObject::connect(d_ptr->m_pWidget->m_pClear, &QPushButton::clicked, d_ptr,
            &RemoteActionNodePrivate::slotClear);
    }
}

QAbstractItemModel* RemoteActionNode::model() const
{
    return d_ptr->m_pModel;
}

std::shared_ptr<sigrok::HardwareDevice> RemoteActionNode::device() const
{
    return d_ptr->m_pDevice;
}

void RemoteActionNode::setDevice(std::shared_ptr<sigrok::HardwareDevice> dev)
{
    //TODO if device and running, ask to stop

    d_ptr->m_pDevice = dev;

    d_ptr->m_Title = QString::fromStdString(dev->model());

    Q_EMIT titleChanged(d_ptr->m_Title);

    auto devm = DeviceModel::instance();

    auto ctx = devm->context();

    setModel(new AquisitionModel(ctx, dev));
}

bool RemoteActionNode::dummy() const{
    // do nothing, the property is a trigger
    return false;
}

void RemoteActionNodePrivate::slotClear()
{
    Q_EMIT q_ptr->aboutToClear();
    m_pModel->clear();
    Q_EMIT q_ptr->cleared();
}
