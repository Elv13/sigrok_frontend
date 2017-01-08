#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class LCDMeterNodePrivate;

class Q_DECL_EXPORT LCDMeterNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit LCDMeterNode(QObject* parent = nullptr);
    virtual ~LCDMeterNode();

    virtual QString title() const override;

    virtual QString id() const override;

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
