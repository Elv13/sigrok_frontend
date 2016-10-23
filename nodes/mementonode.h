#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class MementoNodePrivate;

class MementoNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* mementoModel READ mementoModel NOTIFY mementoModelChanged USER true);

    Q_INVOKABLE explicit MementoNode(QObject* parent = nullptr);
    virtual ~MementoNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    QAbstractItemModel* mementoModel() const;

Q_SIGNALS:
    void mementoModelChanged(const QAbstractItemModel* m);

private:
    MementoNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MementoNode)

};
Q_DECLARE_METATYPE(MementoNode*)
