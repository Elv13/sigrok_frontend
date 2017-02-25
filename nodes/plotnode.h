#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class QwtPlotNodePrivate;

class Q_DECL_EXPORT QwtPlotNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("qwtplot_node", "Plot", "", /*Tags:*/ "Graph", "Chart")
public:
    Q_PROPERTY(bool replot WRITE replot USER true)

    Q_INVOKABLE explicit QwtPlotNode(AbstractSession* sess);
    virtual ~QwtPlotNode();

    virtual QWidget* widget() const override;

    void replot(bool=true);

    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteWidgetType() const override;
    virtual QString remoteModelName() const override;

private:
    QwtPlotNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(QwtPlotNode)

};
Q_DECLARE_METATYPE(QwtPlotNode*)
