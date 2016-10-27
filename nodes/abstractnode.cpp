#include "abstractnode.h"


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
