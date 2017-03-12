#pragma once

#include <QtCore/QAbstractItemModel>

class XmlSchemaBuilderModelPrivate;

class XmlSchemaBuilderModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit XmlSchemaBuilderModel();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;

private:
    XmlSchemaBuilderModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(XmlSchemaBuilderModel)
};
