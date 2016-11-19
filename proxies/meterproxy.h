#pragma once

#include <QtCore/QIdentityProxyModel>

class MeterProxyPrivate;

class Q_DECL_EXPORT MeterProxy : public QIdentityProxyModel
{
    Q_OBJECT
public:
    explicit MeterProxy(QObject* parent = nullptr);

    virtual int columnCount(const QModelIndex& parent = {}) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent = {}) const override;
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    int mainColumn() const;
    bool isColumntSelected(int index) const;

Q_SIGNALS:
    void mainChanged(int main);
    void columnEnabled(int col, bool value);

private:
    MeterProxyPrivate* d_ptr;
    Q_DECLARE_PRIVATE(MeterProxy)
};
