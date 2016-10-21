#pragma once

#include <functional>

#include <QtCore/QIdentityProxyModel>

class QAbstractItemView;

class RangeProxyPrivate;

class RangeProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit RangeProxy(QObject* parent = nullptr);

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int extraColumnCount() const;
    void setExtraColumnCount(int value);

    void setColumnWidgetFactory(int col, std::function<QWidget*(int)> w);

    void setWidget(QAbstractItemView* widget);

Q_SIGNALS:
    void mainChanged(int main);
    void columnEnabled(int col, bool value);

private:
    RangeProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RangeProxy)
};
