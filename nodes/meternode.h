#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class MeterNodePrivate;

class Q_DECL_EXPORT MeterNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit MeterNode(QObject* parent = nullptr);
    virtual ~MeterNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QWidget* widget() const override;

private:
    MeterNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MeterNode)

};
Q_DECLARE_METATYPE(MeterNode*)
