#pragma once

#include <QtCore/QAbstractListModel>

class LastRowToListProxyPrivate;

/**
 * A simple proxy that turns the last row of a table model into a list.
 * 
 * This is useful for data acquisition for displaying the "current" values.
 */
class Q_DECL_EXPORT LastRowToListProxy : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit LastRowToListProxy(QObject* parent = nullptr);
    virtual ~LastRowToListProxy();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual QModelIndex mapFromSource(const QModelIndex& sourceIndex) const;
    virtual QModelIndex mapToSource(const QModelIndex& proxyIndex) const;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

    bool isUsingHeaderAsDisplayRole() const;
    void setUsingHeaderAsDisplayRole(bool value);

    void setSourceModel(QAbstractItemModel* src);
    QAbstractItemModel* sourceModel() const;

private:
    LastRowToListProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(LastRowToListProxy)
};
