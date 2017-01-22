#include "interfaceserializer.h"

#include <QtCore/QHash>
#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

class InterfaceSerializerPrivate
{
public:
    const QString& m_Name;
    QHash<QString, QJsonObject> m_hById;
};

InterfaceSerializer::InterfaceSerializer(const QString& name) :
    d_ptr(new InterfaceSerializerPrivate {name, {}})
{
}

QJsonObject InterfaceSerializer::byId(const QString& id) const
{
    if (!d_ptr->m_hById.contains(id))
        return {};

    return d_ptr->m_hById[id];
}

QHash<QString, QJsonObject> InterfaceSerializer::elements() const
{
    return d_ptr->m_hById;
}

void InterfaceSerializer::add(const QString& uid)
{
    if (d_ptr->m_hById.contains(uid))
        return;

    QJsonObject o;
    writeWidget(o, uid);
    Q_ASSERT(!o["uid"].toString().isEmpty());
    d_ptr->m_hById[uid] = o;
}

void InterfaceSerializer::rename(const QString& id, const QString& newName)
{
    // nothing
}

void InterfaceSerializer::writeWidget(QJsonObject &parent, const QString& id) const
{

}

void InterfaceSerializer::write(QJsonObject &parent) const
{
    QJsonArray arr;

    QHash<QString, QJsonObject>::const_iterator i;

    for (i = d_ptr->m_hById.constBegin(); i != d_ptr->m_hById.constEnd(); ++i) {
        QJsonObject o = i.value();

        o["uid"] = i.key();
        writeWidget(o, i.key());

        arr.append(o);
    }
    parent["widgets"] = arr;
}

void InterfaceSerializer::read(const QJsonObject &parent)
{
    QJsonArray arr = parent["widgets"].toArray();

    for (int i = 0; i < arr.size(); ++i) {
        QJsonObject o = arr[i].toObject();
        const QString name = o["uid"].toString();
        Q_ASSERT(!name.isEmpty());
        d_ptr->m_hById[name] = o;
    }
}
