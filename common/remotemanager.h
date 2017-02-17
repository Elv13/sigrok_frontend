# pragma once

#include <QtCore/QAbstractListModel>
#include <QtCore/QSortFilterProxyModel>

class RemoteManagerPrivate;

class QRemoteObjectHost;

class RemoteObjectList : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit RemoteObjectList(const QByteArray& typeName, QObject* parent = Q_NULLPTR);

    virtual bool filterAcceptsRow(int source_row, const QModelIndex& source_parent) const override;

    QAbstractItemModel* getModel(const QModelIndex& idx) const;

private:
    QByteArray m_Type;
};

class RemoteManager final : public QAbstractListModel
{
    Q_OBJECT
public:
    static RemoteManager* instance();

    virtual QVariant data(const QModelIndex& idx, int role) const override;
    virtual int rowCount(const QModelIndex& parent = {}) const override;
    virtual bool setData(const QModelIndex& idx, const QVariant& value, int role) override;

    QRemoteObjectHost* host() const;

    void addObject(QObject* o, const QString& title);
    void addModel(QAbstractItemModel* o, const QVector<int>& role, const QString& title);

    QAbstractItemModel* getModel(const QModelIndex& idx) const;

private:
    explicit RemoteManager();
    virtual ~RemoteManager();

    RemoteManagerPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteManager)
};
