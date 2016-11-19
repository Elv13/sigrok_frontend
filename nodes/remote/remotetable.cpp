#include "remotetable.h"

#include <QRemoteObjectHost>

#include "common/remotemanager.h"

class RemoteTablePrivate final : public QObject
{
public:

    RemoteTable* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

RemoteTable::RemoteTable(QObject* parent) : ProxyNode(parent), d_ptr(new RemoteTablePrivate())
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &RemoteTablePrivate::slotModelChanged);
}

RemoteTable::~RemoteTable()
{
    delete d_ptr;
}

QString RemoteTable::title() const
{
    return "Remote table";
}

QString RemoteTable::id() const
{
    return QStringLiteral("remotetable_node");
}

void RemoteTable::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* RemoteTable::widget() const
{
    return nullptr;
}

void RemoteTablePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)

    QVector<int> roles;
    roles << Qt::DisplayRole << Qt::BackgroundRole;

    auto node2 = RemoteManager::instance()->host();
    bool worked = node2->enableRemoting(newModel, QStringLiteral("RemoteModel"), roles);
}
