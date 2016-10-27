#pragma once

#include "../abstractnode.h"

#include <QtCore/QObject>

class RemoteTablePrivate;

class RemoteTable : public AbstractNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit RemoteTable(QObject* parent = nullptr);
    virtual ~RemoteTable();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    RemoteTablePrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteTable)

};
Q_DECLARE_METATYPE(RemoteTable*)
