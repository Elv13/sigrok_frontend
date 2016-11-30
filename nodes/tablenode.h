#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TableNodePrivate;

class Q_DECL_EXPORT TableNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit TableNode(QObject* parent = nullptr);
    virtual ~TableNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QWidget* widget() const override;

//     virtual void connectTo(AbstractNode* other, QAbstractItemModel* model, int col) override;

private:
    TableNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TableNode)

};
Q_DECLARE_METATYPE(TableNode*)
