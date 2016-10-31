#pragma once


#include "columnproxy.h"

class QAbstractItemView;

class RangeProxyPrivate;

/**
 * Convert the original model into a tree with the source columns as root
 * indexes and configurable ranges for each column as children.
 * 
 * This is useful to configure a filter or modifier proxy acting on the
 * original values.
 */
class RangeProxy : public ColumnProxy
{
    Q_OBJECT
public:
    enum class Role {
        RANGE_DELIMITER = ColumnProxy::Role::USER_ROLE,
        RANGE_VALUE
    };

    enum class Delimiter {
        ANY,
        EQUAL,
        NOT_EQUAL,
        LESSER,
        GREATER,
        LESSER_EQUAL,
        GREATER_EQUAL,
    };

    explicit RangeProxy(QObject* parent = nullptr);

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual QModelIndex parent(const QModelIndex& idx) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual void setSourceModel(QAbstractItemModel* source) override;

    QModelIndex matchSourceIndex(const QModelIndex& srcIdx) const;

    int extraColumnCount() const;
    void setExtraColumnCount(int value);

//     void setColumnWidgetFactory(int col, bool isRange, std::function<QWidget*(int)> w);

//     void setWidget(QAbstractItemView* widget);

    QAbstractItemModel* delimiterModel() const;

Q_SIGNALS:
    void mainChanged(int main);
    void columnEnabled(int col, bool value);

private:
    RangeProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RangeProxy)
};

Q_ENUMS(RangeProxy::Delimiter)
Q_DECLARE_METATYPE(RangeProxy::Delimiter)
