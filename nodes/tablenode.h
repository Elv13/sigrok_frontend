#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TableNodePrivate;

class TableNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit TableNode(QObject* parent = nullptr);
    virtual ~TableNode();

    virtual QString title() const;

    virtual QWidget* widget() const;

//     virtual void connectTo(AbstractNode* other, QAbstractItemModel* model, int col) override;

private:
    TableNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TableNode)

};
Q_DECLARE_METATYPE(TableNode*)
