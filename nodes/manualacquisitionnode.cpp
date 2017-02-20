#include "manualacquisitionnode.h"

#include "widgets/acquisition.h"
#include "sigrokd/acquisitionmodel.h"

#include <QtCore/QDebug>

class ManualAcquisitionNodePrivate
{
public:
    Acquisition* m_pWidget {nullptr};

};

ManualAcquisitionNode::ManualAcquisitionNode(AbstractSession* sess) :
    AcquisitionNode(sess), d_ptr(new ManualAcquisitionNodePrivate())
{
    
}

ManualAcquisitionNode::~ManualAcquisitionNode()
{
    delete d_ptr;
}

QString ManualAcquisitionNode::title() const
{
    return QStringLiteral("Manual acquisition");
}

QString ManualAcquisitionNode::id() const
{
    return QStringLiteral("manualacquisition_node");
}

void ManualAcquisitionNode::write(QJsonObject &parent) const
{
    AcquisitionNode::write(parent);
}

QWidget* ManualAcquisitionNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = (Acquisition*) AcquisitionNode::widget();
        d_ptr->m_pWidget->setShowAcquire(true);
        connect(d_ptr->m_pWidget, SIGNAL(acquired()), this,SLOT(acquireSample()));
    }

    return d_ptr->m_pWidget;
}

void ManualAcquisitionNode::acquireSample(bool)
{
    if ((!aqModel()) || !aqModel()->addLastSample())
        qWarning() << "Sample failed: There is none yet";
}

void ManualAcquisitionNode::acquireSampleAs(const QString& name)
{
    if (aqModel() && aqModel()->addLastSample()) {
        const auto i = aqModel()->index(aqModel()->rowCount()-1,0);
        Q_ASSERT(i.isValid());
        aqModel()->setData(i, name, Qt::EditRole);
    }
}

void ManualAcquisitionNode::setDevice(SigrokDevice* dev)
{
    if (device() == dev)
        return;

    AcquisitionNode::setDevice(dev);

    aqModel()->setMode(AcquisitionModel::Mode::MANUAL);
}

QStringList ManualAcquisitionNode::searchTags() const
{
    static QStringList l {
        QStringLiteral("hardware"),
        QStringLiteral("sampling"),
        QStringLiteral("sample"),
    };

    return l;
}
