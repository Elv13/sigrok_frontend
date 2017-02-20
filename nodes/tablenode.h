#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TableNodePrivate;

class Q_DECL_EXPORT TableNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit TableNode(AbstractSession* sess);
    virtual ~TableNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QWidget* widget() const override;

    virtual QString remoteModelName() const override;
    virtual QString remoteWidgetType() const override;

    virtual QStringList searchTags() const override;

//     virtual void connectTo(AbstractNode* other, QAbstractItemModel* model, int col) override;

private:
    TableNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TableNode)

};
Q_DECLARE_METATYPE(TableNode*)
