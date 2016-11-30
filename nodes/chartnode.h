#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class ChartNodePrivate;

class Q_DECL_EXPORT ChartNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ChartNode(QObject* parent = nullptr);
    virtual ~ChartNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    ChartNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChartNode)

};
Q_DECLARE_METATYPE(ChartNode*)
