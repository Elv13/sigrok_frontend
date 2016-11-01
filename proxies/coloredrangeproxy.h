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
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QAbstractItemModel* filteredModel() const;

private:
    ColoredRangeProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColoredRangeProxy)
};
