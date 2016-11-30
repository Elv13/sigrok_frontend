#pragma once

#include <QtCore/QIdentityProxyModel>

class TailProxyPrivate;

/**
 * Keep the last `n` rows of a model.
 *
 * Useful with models with a lot of irrelevant rows or models with high
 * throughput.
 */
class Q_DECL_EXPORT TailProxy : public QAbstractProxyModel
{
    Q_OBJECT
public:
    Q_PROPERTY(int maximum READ maximum WRITE setMaximum NOTIFY maximumChanged)

    explicit TailProxy(QObject* parent = nullptr);
    virtual ~TailProxy();

    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const override;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;
    virtual void setSourceModel(QAbstractItemModel *sourceModel) override;

    bool isLimited() const;

    bool isCurrentlyLimited() const;

    int maximum() const;

public Q_SLOTS:
    void setMaximum(int max);
    void setLimited(bool limit);

Q_SIGNALS:
    void maximumChanged(int);

private:
    TailProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(TailProxy)
};
