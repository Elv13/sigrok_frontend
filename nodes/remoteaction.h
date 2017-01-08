#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class RemoteActionNodePrivate;

class Q_DECL_EXPORT RemoteActionNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit RemoteActionNode(QObject* parent = nullptr);
    virtual ~RemoteActionNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteModelName() const override;
    virtual QString remoteWidgetType() const override;

    virtual QWidget* widget() const override;

private:
    RemoteActionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteActionNode)

};
Q_DECLARE_METATYPE(RemoteActionNode*)
