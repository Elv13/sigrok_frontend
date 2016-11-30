#include "manualaquisitionnode.h"

#include "widgets/aquisition.h"
#include "sigrokd/aquisitionmodel.h"

#include <QtCore/QDebug>

class ManualAquisitionNodePrivate
{
public:
    Aquisition* m_pWidget {nullptr};

};

ManualAquisitionNode::ManualAquisitionNode(QObject* parent) :
    AquisitionNode(parent), d_ptr(new ManualAquisitionNodePrivate())
{
    
}

ManualAquisitionNode::~ManualAquisitionNode()
{
    
}

QString ManualAquisitionNode::title() const
{
    return "Manual aquisition";
}

QString ManualAquisitionNode::id() const
{
    return "manualaquisition_node";
}

void ManualAquisitionNode::write(QJsonObject &parent) const
{
    AquisitionNode::write(parent);
}

QWidget* ManualAquisitionNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = (Aquisition*) AquisitionNode::widget();
        d_ptr->m_pWidget->setShowAcquire(true);
        connect(d_ptr->m_pWidget, SIGNAL(aquired()), this,SLOT(aquireSample()));
    }

    return d_ptr->m_pWidget;
}

void ManualAquisitionNode::aquireSample(bool)
{
    if ((!aqModel()) || !aqModel()->addLastSample())
        qWarning() << "Sample failed: There is none yet";
}

void ManualAquisitionNode::setDevice(SigrokDevice* dev)
{
    if (device() == dev)
        return;

    AquisitionNode::setDevice(dev);

    aqModel()->setMode(AquisitionModel::Mode::MANUAL);
}
