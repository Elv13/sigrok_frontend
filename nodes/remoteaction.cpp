#include "remoteaction.h"

#include "widgets/controls.h"

#include "common/pagemanager.h"
#include "common/abstractsession.h"

#include "../models/remotewidgets.h"

#include "remotemanager.h"

#include <QDebug>

#include <QtCore/QTimer>

class RemoteActionNodePrivate : public QObject
{
    Q_OBJECT
public:

    RemoteWidgets* m_pModel{nullptr};
    Controls m_Current;
    QString m_Id;
    bool m_IsRegistered {false};

};

RemoteActionNode::RemoteActionNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new RemoteActionNodePrivate())
{
    d_ptr->m_pModel = new RemoteWidgets(this);
    d_ptr->m_pModel->setObjectName(QStringLiteral("RemoteControl"));
    d_ptr->m_Current.setModel(d_ptr->m_pModel->clientModel());

    QTimer::singleShot(0, [this]() {
        session()->pages()->addPage(this, &d_ptr->m_Current, title(), uid());
    });
}

RemoteActionNode::~RemoteActionNode()
{
    delete d_ptr;
}

void RemoteActionNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* RemoteActionNode::widget() const
{
    return nullptr;
}

QString RemoteActionNode::remoteModelName() const
{
    widget();
    static int count = 0;

    if (d_ptr->m_Id.isEmpty())
        d_ptr->m_Id = QStringLiteral("remoteaction")+QString::number(count++);

    if (!d_ptr->m_IsRegistered)
        RemoteManager::instance()->addModel(d_ptr->m_pModel, {
            Qt::DisplayRole,
            Qt::EditRole,
        }, d_ptr->m_Id);

    d_ptr->m_IsRegistered = true;

    return d_ptr->m_Id;
}

QString RemoteActionNode::remoteWidgetType() const
{
    return id();
}

bool RemoteActionNode::createSourceSocket(const QString& name)
{
    return d_ptr->m_pModel->addRow(name);
}

QAbstractItemModel* RemoteActionNode::sourceModel() const
{
    return d_ptr->m_pModel;
}

#include <remoteaction.moc>
