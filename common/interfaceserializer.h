#pragma once

#include <QtCore/QJsonObject>

class InterfaceSerializerPrivate;

class InterfaceSerializer
{
public:
    explicit InterfaceSerializer(const QString& name);

    virtual void reflow() const = 0;
    QJsonObject byId(const QString& id) const;
    void add(const QString& uid);

    QHash<QString, QJsonObject> elements() const;

    virtual void write(QJsonObject &parent) const;
    virtual void read(const QJsonObject &parent);

protected:
    virtual void writeWidget(QJsonObject &parent, const QString& id) const;
private:
    InterfaceSerializerPrivate* d_ptr;
};
