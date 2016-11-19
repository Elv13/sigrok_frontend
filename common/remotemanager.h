# pragma once

class RemoteManagerPrivate;

class QRemoteObjectHost;

class RemoteManager final
{
public:
    static RemoteManager* instance();

    QRemoteObjectHost* host() const;

private:
    explicit RemoteManager();

    RemoteManagerPrivate* d_ptr;
};
