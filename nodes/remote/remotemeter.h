#pragma once

#include "../proxynode.h"

#include <QtCore/QObject>

class RemoteMeterPrivate;

class RemoteMeter : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("remotemeter_node", "Remote meter", "", /*Tags:*/ "")

public:

    Q_INVOKABLE explicit RemoteMeter(AbstractSession* sess);
    virtual ~RemoteMeter();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    RemoteMeterPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteMeter)

};
Q_DECLARE_METATYPE(RemoteMeter*)
