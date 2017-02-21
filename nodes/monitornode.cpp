#include "monitornode.h"

#include <QtCore/QDateTime>
#include <QtCore/QDebug>

#include "widgets/monitor.h"

class MonitorNodePrivate final : public QObject
{
    Q_OBJECT
public:
    MonitorNodePrivate(QObject* parent) : QObject(parent) {}

    Monitor* m_pWidget {nullptr};

    MonitorNode* q_ptr;
};

MonitorNode::MonitorNode(AbstractSession* sess) : AbstractNode(sess), d_ptr(new MonitorNodePrivate(this))
{
    d_ptr->q_ptr = this;
}

MonitorNode::~MonitorNode()
{
    delete d_ptr;
}

void MonitorNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

void MonitorNode::read(const QJsonObject &parent)
{
}

QWidget* MonitorNode::widget() const
{
    if (!d_ptr->m_pWidget)
        d_ptr->m_pWidget = new Monitor();

    return d_ptr->m_pWidget;
}

void MonitorNode::setValue(const QVariant& v)
{
    if (d_ptr->m_pWidget) {
        d_ptr->m_pWidget->setType(v.typeName());
        d_ptr->m_pWidget->setTime(QDateTime::currentDateTime().toString());
        d_ptr->m_pWidget->setContent(v.toString());
    }
}

QVariant MonitorNode::value() const
{
    return {};
}

#include <monitornode.moc>
