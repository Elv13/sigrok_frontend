#pragma once

#include <QtCore/QAbstractListModel>

class ColumnProxyPrivate;

/**
 * Take a source model and convert the headers into a list model.
 *
 * This is useful in combination with the KRearrangeColumnsProxyModel to select
 * and KCheckableProxyModel visible columns.
 */
class ColumnProxy : public QAbstractListModel
{
    Q_OBJECT
public:
    enum class Role {
        SOURCE_COLUMN_INDEX = Qt::UserRole+1,
        SOURCE_COLUMN_NAME
    };

    explicit ColumnProxy(QObject* parent = nullptr);
    ColumnProxy(QAbstractItemModel* source);
    virtual ~ColumnProxy();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;

    void setSourceModel(QAbstractItemModel* source);
    QAbstractItemModel* sourceModel() const;

private:
    ColumnProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColumnProxy)
};
