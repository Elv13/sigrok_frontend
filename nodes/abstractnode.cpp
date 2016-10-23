#include "abstractnode.h"


void AbstractNode::write(QJsonObject &parent) const
{
    QJsonObject self;

    parent[ "id"    ] = id();
    parent[ "title" ] = title();

}
