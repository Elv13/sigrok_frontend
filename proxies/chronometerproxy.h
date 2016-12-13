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
class Q_DECL_EXPORT ChronometerProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    enum ExtraColumns {
        AUTO      = 0 << 0,
        MS_DELTA  = 1 << 0,
        MS_EPOCH  = 1 << 1,
        TIMESTAMP = 1 << 2,
        DATE_TIME = 1 << 3,
    };

    explicit ChronometerProxy(QObject* parent = nullptr);
    virtual ~ChronometerProxy();

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    /// Use a row column instead of std::chrono
    void setTimeSourceColumn(int col);

    /// Use a role instead of std::chrono
    void setTimeSourceRole(int role);

    void setExtraColumns(ExtraColumns cols);
    int extraColumns() const;

private:
    ChronometerProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChronometerProxy)
};
