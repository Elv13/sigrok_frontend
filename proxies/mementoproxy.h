#pragma once

#include <QtCore/QAbstractItemModel>
#include <QtCore/QJsonObject>

class MementoProxyPrivate;

/**
 * Create a copy of the a model.
 *
 * It uses the ::roleNames() method to get all used roles. The roles have to
 * be defined by the first root QModelIndex() or they will be ignored further
 * on.
 *
 * All source QVariants are kept, so either use smart pointers or avoid freeing
 * the (potential) QObject memory to avoid crashes.
 *
 * This model has basic support for JSon serialization (for all roles) and
 * CSV (for list or table using the Qt::DisplayRole role).
 * 
 * Limitation:
 *  * Only support custom roles if they are defined in roleNames
 *  * Always assume all the indices have the same roles (for performance)
 *  * Can't serialize objects when saving.
 *  * Always make a full copy, so takes a lot of memory.
 *  * Copying is done in the model thread, so it is blocking
 *  * This model currently doesn't support incremental updates
 *
 */
class Q_DECL_EXPORT MementoProxy : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit MementoProxy(QObject* parent = nullptr);
    MementoProxy(const QJsonObject& json, QObject* parent = nullptr);
    virtual ~MementoProxy();

    void setSourceModel(QAbstractItemModel* model);
    QAbstractItemModel* sourceModel() const;

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QModelIndex index(int row, int column, const QModelIndex& parent ={}) const override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;

    QJsonObject toJson() const;

public Q_SLOTS:
    void sync();

private:
    MementoProxyPrivate* d_ptr;
};
