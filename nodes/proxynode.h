#pragma once

#include "common/abstractnode.h"

#include <QtCore/QObject>

class QAbstractItemModel;

class ProxyNodePrivate;

class Q_DECL_EXPORT ProxyNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("proxy_node", "Proxy node", "", /*Tags:*/ "")
public:
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel NOTIFY modelChanged USER true)

    Q_INVOKABLE explicit ProxyNode(AbstractSession* sess);
    virtual ~ProxyNode();

    virtual QWidget* widget() const override;

    void setModel(QAbstractItemModel* m);
    QAbstractItemModel* model() const;

Q_SIGNALS:
    void modelChanged(QAbstractItemModel* newModel, QAbstractItemModel* oldModel);

private:
    ProxyNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ProxyNode)

};
Q_DECLARE_METATYPE(ProxyNode*)
