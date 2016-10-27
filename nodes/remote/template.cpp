#include "remotetable.h"


class RemoteTablePrivate final : public QObject
{
public:

    RemoteTable* q_ptr;
};

RemoteTable::RemoteTable(QObject* parent) : AbstractNode(parent), d_ptr(new RemoteTablePrivate())
{
    d_ptr->q_ptr = this;
}

RemoteTable::~RemoteTable()
{
    delete d_ptr;
}

QString RemoteTable::title() const
{
    return "Multiplexer";
}

QString RemoteTable::id() const
{
    return QStringLiteral("multiplexer_node");
}

void RemoteTable::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* RemoteTable::widget() const
{
    return nullptr;
}
