#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class CurveChartNodePrivate;

class Q_DECL_EXPORT CurveChartNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("curvedchart_node", "Chart", "", /*Tags:*/ "Graph")
public:
    Q_INVOKABLE explicit CurveChartNode(AbstractSession* sess);
    virtual ~CurveChartNode();

    virtual QWidget* widget() const override;


    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteWidgetType() const override;
    virtual QString remoteModelName() const override;

private:
    CurveChartNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(CurveChartNode)

};
Q_DECLARE_METATYPE(CurveChartNode*)
