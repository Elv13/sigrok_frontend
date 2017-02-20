#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class ChartNodePrivate;

class Q_DECL_EXPORT ChartNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("chart_node", "Chart", "", /*Tags:*/ "graph")
public:
    Q_INVOKABLE explicit ChartNode(AbstractSession* sess);
    virtual ~ChartNode();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;


private:
    ChartNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChartNode)

};
Q_DECLARE_METATYPE(ChartNode*)
