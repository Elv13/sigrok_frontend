#pragma once

#include <QtCore/QAbstractListModel>

class QMainWindow;

class WidgetGroupModelPrivate;

class WidgetGroupModel : public QAbstractListModel
{
    Q_OBJECT
public:
    explicit WidgetGroupModel(QObject* parent = nullptr);
    virtual ~WidgetGroupModel();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
//     virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;

    QString addGroup(QMainWindow* w, const QString& name, const QString& id);
    QMainWindow* mainWindow(const QModelIndex& idx) const;

private:
    WidgetGroupModelPrivate* d_ptr;
    Q_DECLARE_PRIVATE(WidgetGroupModel)
};
