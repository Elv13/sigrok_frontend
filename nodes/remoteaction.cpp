#include "remoteaction.h"

#include "widgets/controls.h"
#include "widgets/controlschooser.h"

#include "common/pagemanager.h"
#include "common/abstractsession.h"

#include "remotemanager.h"

#include <QDebug>

#include <QtCore/QTimer>

class RemoteActionNodePrivate : public QObject
{
    Q_OBJECT
public:
    Controls m_Current;
    ControlsChooser* m_pControlCW {nullptr};
    QAbstractItemModel* m_pSource {nullptr};
    QString m_Id;
    bool m_IsRegistered {false};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

RemoteActionNode::RemoteActionNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new RemoteActionNodePrivate())
{
    QTimer::singleShot(0, [this]() {
        session()->pages()->addPage(this, &d_ptr->m_Current, title(), uid());
    });

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &RemoteActionNodePrivate::slotModelChanged);
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
    if (!d_ptr->m_pControlCW) {
        d_ptr->m_pControlCW = new ControlsChooser();
        d_ptr->m_Current.setModel(d_ptr->m_pControlCW->currentModel());
    }

    return d_ptr->m_pControlCW;
}

QString RemoteActionNode::remoteModelName() const
{
    widget();
    static int count = 0;

    if (d_ptr->m_Id.isEmpty())
        d_ptr->m_Id = QStringLiteral("remoteaction")+QString::number(count++);

    if (!d_ptr->m_IsRegistered)
        RemoteManager::instance()->addModel(d_ptr->m_pControlCW->currentModel(), {
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


void RemoteActionNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_pSource = newModel;
    m_Current.setModel(newModel);
}

#include <remoteaction.moc>
