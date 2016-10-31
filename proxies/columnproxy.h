#pragma once

#include <QtCore/QAbstractListModel>

class ColumnProxyPrivate;

/**
 * Take a source model and convert the headers into a list model.
 *
 * This is useful in combination with the KRearrangeColumnsProxyModel to select
 * and KCheckableProxyModel visible columns.
 */
class ColumnProxy : public QAbstractItemModel
{
    Q_OBJECT
public:
    enum class Role {
        SOURCE_COLUMN_INDEX = Qt::UserRole+1,
        SOURCE_COLUMN_NAME,
        USER_ROLE
    };

    explicit ColumnProxy(QObject* parent = nullptr);
    ColumnProxy(QAbstractItemModel* source);
    virtual ~ColumnProxy();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;

    virtual void setSourceModel(QAbstractItemModel* source);
    QAbstractItemModel* sourceModel() const;

private:
    ColumnProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColumnProxy)
};
