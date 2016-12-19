#include "abstractnode.h"

#ifndef DECL_QAbstractItemModel
#define DECL_QAbstractItemModel
Q_DECLARE_STREAM_METATYPE(QAbstractItemModel*)
#endif

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
