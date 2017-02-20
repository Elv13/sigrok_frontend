#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class MeterNodePrivate;

class Q_DECL_EXPORT MeterNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("meter_node", "Meter", "", /*Tags:*/
        "display",
        "view",
        "lcd",
        "segments",
        "statistics"
    )
public:
    Q_INVOKABLE explicit MeterNode(AbstractSession* sess);
    virtual ~MeterNode();

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QString remoteWidgetType() const override;

    virtual QWidget* widget() const override;

private:
    MeterNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MeterNode)

};
Q_DECLARE_METATYPE(MeterNode*)
