#include "remotecontrols.h"

#include "../models/remotewidgets.h"

class RemoteControlsPrivate final : public QObject
{
public:
    explicit RemoteControlsPrivate(RemoteControls* p) : QObject(p), q_ptr(p) {}

    RemoteWidgets m_Model{this};
    RemoteControls* q_ptr;
};

RemoteControls::RemoteControls(QObject* parent) : AbstractNode(parent), d_ptr(new RemoteControlsPrivate(this))
{
    d_ptr->m_Model.setObjectName("RemoteControl");
}

RemoteControls::~RemoteControls()
{
    delete d_ptr;
}

QString RemoteControls::title() const
{
    return "Control widgets";
}

QString RemoteControls::id() const
{
    return QStringLiteral("remotecontrols_node");
}

void RemoteControls::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* RemoteControls::widget() const
{
    return nullptr;
}

QAbstractItemModel* RemoteControls::sourceModel() const
{
    return &d_ptr->m_Model;
}
