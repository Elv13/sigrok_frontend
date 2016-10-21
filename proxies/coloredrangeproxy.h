#pragma once

#include "rangeproxy.h"

class ColoredRangeProxyPrivate;

class ColoredRangeProxy : public RangeProxy
{
    Q_OBJECT
public:
    explicit ColoredRangeProxy(QObject* parent = nullptr);
    virtual ~ColoredRangeProxy();

    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

    QAbstractItemModel* filteredModel() const;

private:
    ColoredRangeProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColoredRangeProxy)
};
