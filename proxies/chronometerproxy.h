#pragma once

#include <QtCore/QIdentityProxyModel>

class ChronometerProxyPrivate;

/**
 * Record the time between each "rowsInserted" and add the result to a new
 * column.
 * 
 * This is useful for:
 *
 *  * live data sources
 *  * events logging
 *  * data aquisition
 *
 *TODO also compute the mean / rate over the last `n` samples.
 */
class ChronometerProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit ChronometerProxy(QObject* parent = nullptr);
    virtual ~ChronometerProxy();

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

private:
    ChronometerProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChronometerProxy)
};
