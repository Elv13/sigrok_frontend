#include "remotemanager.h"

#include <QRemoteObjectHost>

class RemoteManagerPrivate
{
public:
    mutable QRemoteObjectRegistryHost* m_pReg {nullptr};
    mutable QRemoteObjectHost* m_pHost {nullptr};
    mutable QRemoteObjectHost* m_pHost2 {nullptr};
};

RemoteManager::RemoteManager() : d_ptr(new RemoteManagerPrivate())
{
    
}

RemoteManager* RemoteManager::instance()
{
    auto i = new RemoteManager();

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
    }

    return d_ptr->m_pHost;
}

QRemoteObjectHost* RemoteManager::host2() const
{
    if (!d_ptr->m_pHost2) {
        if (!d_ptr->m_pReg)
            d_ptr->m_pReg = new QRemoteObjectRegistryHost(QUrl(QStringLiteral("tcp://10.10.10.136:2223")));

        d_ptr->m_pHost2 = new QRemoteObjectHost(
            QUrl(QStringLiteral("tcp://10.10.10.136:2225")),
            QUrl(QStringLiteral("tcp://10.10.10.136:2223"))
        );
    }

    return d_ptr->m_pHost2;
}
