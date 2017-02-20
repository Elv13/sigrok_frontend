#include "remotetable.h"

#include <QRemoteObjectHost>

#include "common/remotemanager.h"

class RemoteTablePrivate final : public QObject
{
    Q_OBJECT
public:

    RemoteTable* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

RemoteTable::RemoteTable(AbstractSession* sess) : ProxyNode(sess), d_ptr(new RemoteTablePrivate())
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &RemoteTablePrivate::slotModelChanged);
}

RemoteTable::~RemoteTable()
{
    delete d_ptr;
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

    static QVector<int> roles {
        Qt::DisplayRole,
        Qt::BackgroundRole
    };

    RemoteManager::instance()->addModel(newModel, roles, QStringLiteral("RemoteModel"));
}

QString RemoteTable::remoteModelName() const
{
    return QStringLiteral("remotetable");
}

#include <remotetable.moc>
