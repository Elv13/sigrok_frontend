#pragma once

#include <QtCore/QIdentityProxyModel>

class QItemSelectionModel;

class RowsDeletionProxyPrivate;

/**
 * Add a new column to the model for a "list-remove" button.
 *
 * This proxy avoid adding redundant logic in the source model.
 *
 * It require a QItemSelectionModel to watch for "click" on the remove button.
 */
class RowsDeletionProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit RowsDeletionProxy(QObject* parent = Q_NULLPTR);
    virtual ~RowsDeletionProxy();

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;

    void setSelectionModel(QItemSelectionModel* sm);
    void setIcon(const QVariant& icon);

private:
    RowsDeletionProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RowsDeletionProxy)
};
