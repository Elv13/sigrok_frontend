#include "abstractnode.h"

#include "pagemanager.h"
#include "abstractsession.h"

#ifndef DECL_QAbstractItemModel
#define DECL_QAbstractItemModel
Q_DECLARE_STREAM_METATYPE(QAbstractItemModel*)
#endif

class AbstractNodePrivate : public QObject
{
public:
    QString m_Uid;
    AbstractSession* m_pSession;
};

AbstractNode::AbstractNode(AbstractSession* sess) : QObject(sess),
    d_ptr(new AbstractNodePrivate)
{
    Q_ASSERT(sess);
    d_ptr->m_pSession = sess;
}

AbstractNode::~AbstractNode()
{
    session()->pages()->removePage(this);
    d_ptr->deleteLater();
}

AbstractSession* AbstractNode::session() const
{
    return d_ptr->m_pSession;
}

void AbstractNode::write(QJsonObject &parent) const
{
    QJsonObject self;

    parent[ QStringLiteral("id"   ) ] = id();
    parent[ QStringLiteral("title") ] = title();

}

void AbstractNode::read(const QJsonObject &parent)
{
    Q_UNUSED(parent)
}

bool AbstractNode::createSinkSocket(const QString& name)
{
    Q_UNUSED(name)
    return false;
}

bool AbstractNode::createSourceSocket(const QString& name)
{
    Q_UNUSED(name)
    return false;
}

QString AbstractNode::remoteObjectName() const
{
    return QString();
}

QString AbstractNode::remoteModelName() const
{
    return QString();
}

QString AbstractNode::remoteWidgetType() const
{
    return QString();
}

void AbstractNode::setUid(const QString& uid)
{
    d_ptr->m_Uid = uid;
}

QString AbstractNode::uid() const
{
    Q_ASSERT(!d_ptr->m_Uid.isEmpty());
    return d_ptr->m_Uid;
}

QStringList AbstractNode::searchTags() const
{
    return {};
}
