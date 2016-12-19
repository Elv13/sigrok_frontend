#pragma once

#include <QtCore/QAbstractItemModel>
#include <QtCore/QIdentityProxyModel>

class RemoteWidgetsPrivate;
class RemoteWidgets;
class QJsonObject;

class RemoteWidgetsClient : public QIdentityProxyModel
{
    friend class RemoteWidgets; // for the constructor

    Q_OBJECT

public:
    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
//     virtual bool setData(const QModelIndex &i, const QVariant &value, int role) override;

private:
    explicit RemoteWidgetsClient(RemoteWidgets* src);
};

class RemoteWidgets : public QAbstractListModel
{
    Q_OBJECT
public:
    enum Role {
        VALUE = Qt::UserRole +1,
        MIN_VALUE,
        MAX_VALUE,
        TYPE_NAME,
    };

    enum class Mode {
        CLIENT,
        SERVER,
    };

    explicit RemoteWidgets(QObject* o = nullptr);
    virtual ~RemoteWidgets();

    RemoteWidgetsClient* clientModel() const;

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;
    virtual Qt::ItemFlags flags(const QModelIndex &idx) const override;

    bool addRow(const QModelIndex& idx);
    bool addRow(const QString& name);

    void write(QJsonObject &parent) const;

private:
    RemoteWidgetsPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteWidgets)
};
