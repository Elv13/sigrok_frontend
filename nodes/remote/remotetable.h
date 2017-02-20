#pragma once

#include "../proxynode.h"

#include <QtCore/QObject>

class RemoteTablePrivate;

class RemoteTable : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("remotetable_node", "Remote table", "", /*Tags:*/ "")
public:

    Q_INVOKABLE explicit RemoteTable(AbstractSession* sess);
    virtual ~RemoteTable();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;

    virtual QString remoteModelName() const override;

private:
    RemoteTablePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteTable)

};
Q_DECLARE_METATYPE(RemoteTable*)
