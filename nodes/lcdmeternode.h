#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class LCDMeterNodePrivate;

class Q_DECL_EXPORT LCDMeterNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("lcdmeter_node", "LCD Meter", "", /*Tags:*/
        "display",
        "view",
        "LCD",
        "segments"
    )
public:
    Q_INVOKABLE explicit LCDMeterNode(AbstractSession* sess);
    virtual ~LCDMeterNode();

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QString remoteWidgetType() const override;
    virtual QString remoteModelName() const override;

    virtual QWidget* widget() const override;

private:
    LCDMeterNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(LCDMeterNode)

};
Q_DECLARE_METATYPE(LCDMeterNode*)
