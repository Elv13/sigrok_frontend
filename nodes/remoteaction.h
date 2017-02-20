#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class RemoteActionNodePrivate;

class Q_DECL_EXPORT RemoteActionNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("remoteaction_node", "Controls", "", /*Tags:*/ "")

public:
    Q_INVOKABLE explicit RemoteActionNode(AbstractSession* sess);
    virtual ~RemoteActionNode();

    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteModelName() const override;
    virtual QString remoteWidgetType() const override;

    virtual QWidget* widget() const override;

private:
    RemoteActionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteActionNode)

};
Q_DECLARE_METATYPE(RemoteActionNode*)
