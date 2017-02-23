#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class RemoteActionNodePrivate;

class Q_DECL_EXPORT RemoteActionNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("remoteaction_node", "Controls", "", /*Tags:*/
        "buttons",
        "sliders",
    )

public:
    Q_INVOKABLE explicit RemoteActionNode(AbstractSession* sess);
    virtual ~RemoteActionNode();

    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteModelName() const override;
    virtual QString remoteWidgetType() const override;

    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}
    virtual QAbstractItemModel* sourceModel() const override;
    virtual bool createSourceSocket(const QString& name) override;

private:
    RemoteActionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteActionNode)

};
Q_DECLARE_METATYPE(RemoteActionNode*)
