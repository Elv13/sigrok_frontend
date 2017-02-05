#include "remotemanager.h"

#include <QRemoteObjectHost>

struct RObject
{
    QObject* m_pObject;
    QAbstractItemModel* m_pModel;
    QString m_Name;
    bool m_IsExported;
};

class RemoteManagerPrivate
{
public:
    mutable QRemoteObjectRegistryHost* m_pReg {nullptr};
    mutable QRemoteObjectHost* m_pHost {nullptr};

    mutable QList<RObject*> m_lObjects;
};

RemoteManager::RemoteManager() : d_ptr(new RemoteManagerPrivate())
{
    
}

RemoteManager* RemoteManager::instance()
{
    static auto i = new RemoteManager();

    return i;
}

QRemoteObjectHost* RemoteManager::host() const
{
    if (!d_ptr->m_pHost) {
        if (!d_ptr->m_pReg)
            d_ptr->m_pReg = new QRemoteObjectRegistryHost(QUrl(QStringLiteral("tcp://10.10.10.136:2223")));

        d_ptr->m_pHost = new QRemoteObjectHost(
            QUrl(QStringLiteral("tcp://10.10.10.136:2224")),
            QUrl(QStringLiteral("tcp://10.10.10.136:2223"))
        );

        // Export itself
        const_cast<RemoteManager*>(this)->addModel(
            const_cast<RemoteManager*>(this),
            {
                Qt::DisplayRole,
                Qt::UserRole,
            },
            QStringLiteral("RemoteManager")
        );
    }

    return d_ptr->m_pHost;
}

void RemoteManager::addObject(QObject* o, const QString& title)
{
    auto i = new RObject {
        o, nullptr, title, true
    };

    if (!host()->enableRemoting(o)) {
        qWarning() << "Remote widgets sharing failed";
    }

    beginInsertRows({}, d_ptr->m_lObjects.size(), d_ptr->m_lObjects.size());
    d_ptr->m_lObjects << i;
    endInsertRows();
}

void RemoteManager::addModel(QAbstractItemModel* m, const QVector<int>& roles, const QString& title)
{
    host()->enableRemoting(m, title, roles);

    auto i = new RObject {
        nullptr, m, title, true
    };

    beginInsertRows({}, d_ptr->m_lObjects.size(), d_ptr->m_lObjects.size());
    d_ptr->m_lObjects << i;
    endInsertRows();
}

QVariant RemoteManager::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto i = d_ptr->m_lObjects[idx.row()];

    switch (role) {
        case Qt::DisplayRole:
            return i->m_Name;
        case Qt::UserRole:
            return i->m_pModel ? i->m_pModel->metaObject()->className() : i->m_pObject->metaObject()->className();
    }

    return {};
}

int RemoteManager::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lObjects.size();
}

bool RemoteManager::setData(const QModelIndex& idx, const QVariant& value, int role)
{
    Q_UNUSED(idx)
    Q_UNUSED(value)
    Q_UNUSED(role)
    return false;
}

QAbstractItemModel* RemoteManager::getModel(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return Q_NULLPTR;

    return d_ptr->m_lObjects[idx.row()]->m_pModel;
}

RemoteObjectList::RemoteObjectList(const QByteArray& typeName, QObject* parent):
    QSortFilterProxyModel(parent), m_Type(typeName)
{
    setSourceModel(RemoteManager::instance());
}

bool RemoteObjectList::filterAcceptsRow(int r, const QModelIndex& source_parent) const
{
    const auto idx = sourceModel()->index(r, 0, source_parent);

    return idx.data(Qt::UserRole) == m_Type;
}

QAbstractItemModel* RemoteObjectList::getModel(const QModelIndex& idx) const
{
    return RemoteManager::instance()->getModel(mapToSource(idx));
}
