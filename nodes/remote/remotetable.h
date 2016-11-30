#pragma once

#include "../proxynode.h"

#include <QtCore/QObject>

class RemoteTablePrivate;

class RemoteTable : public ProxyNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit RemoteTable(QObject* parent = nullptr);
    virtual ~RemoteTable();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    RemoteTablePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteTable)

};
Q_DECLARE_METATYPE(RemoteTable*)
