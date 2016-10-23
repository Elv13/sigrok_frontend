#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class QAbstractItemModel;

class ProxyNodePrivate;

class ProxyNode : public AbstractNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel NOTIFY modelChanged USER true);

    Q_INVOKABLE explicit ProxyNode(QObject* parent = nullptr);
    virtual ~ProxyNode();

    virtual QString title() const;

    virtual QString id() const override;

    virtual QWidget* widget() const;

    void setModel(QAbstractItemModel* m);
    QAbstractItemModel* model() const;

Q_SIGNALS:
    void modelChanged(QAbstractItemModel* newModel, QAbstractItemModel* oldModel);

private:
    ProxyNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ProxyNode)

};
Q_DECLARE_METATYPE(ProxyNode*)
