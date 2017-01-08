#include "abstractnode.h"

#include "pagemanager.h"

#ifndef DECL_QAbstractItemModel
#define DECL_QAbstractItemModel
Q_DECLARE_STREAM_METATYPE(QAbstractItemModel*)
#endif

class AbstractNodePrivate
{
public:
    QString m_Uid;
};

AbstractNode::AbstractNode(QObject* parent) : QObject(parent),
    d_ptr(new AbstractNodePrivate)
{
    
}

AbstractNode::~AbstractNode()
{
    PageManager::instance()->removePage(this);
}

void AbstractNode::write(QJsonObject &parent) const
{
    QJsonObject self;

    parent[ "id"    ] = id();
    parent[ "title" ] = title();

}

void AbstractNode::read(const QJsonObject &parent)
{
    Q_UNUSED(parent)
}

bool AbstractNode::createSocket(const QString& name)
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
