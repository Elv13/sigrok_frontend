#pragma once

#include <QtCore/QAbstractItemModel>

class RemoteWidgetsPrivate;

class RemoteWidgets : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit RemoteWidgets(QObject* o = nullptr);
    virtual ~RemoteWidgets();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

private:
    RemoteWidgetsPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteWidgets)
};
