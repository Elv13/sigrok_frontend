#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class CurveChartNodePrivate;

class Q_DECL_EXPORT CurveChartNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit CurveChartNode(QObject* parent = nullptr);
    virtual ~CurveChartNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteWidgetType() const override;
    virtual QString remoteModelName() const override;

private:
    CurveChartNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(CurveChartNode)

};
Q_DECLARE_METATYPE(CurveChartNode*)
