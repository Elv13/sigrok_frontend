#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class MeterNodePrivate;

class MeterNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit MeterNode(QObject* parent = nullptr);
    virtual ~MeterNode();

    virtual QString title() const;

    virtual QWidget* widget() const;

private:
    MeterNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MeterNode)

};
Q_DECLARE_METATYPE(MeterNode*)
