#include "currentvalues.h"

#include "../proxies/lastrowtolistproxy.h"


class CurrentValuesPrivate final : public QObject
{
public:
    LastRowToListProxy m_Proxy{this};

    CurrentValues* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

CurrentValues::CurrentValues(QObject* parent) : ProxyNode(parent), d_ptr(new CurrentValuesPrivate())
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &CurrentValuesPrivate::slotModelChanged);
}

CurrentValues::~CurrentValues()
{
    delete d_ptr;
}

QString CurrentValues::title() const
{
    return "Current Values";
}

QString CurrentValues::id() const
{
    return QStringLiteral("currentvalues_node");
}

QAbstractItemModel* CurrentValues::sourceModel() const
{
    return &d_ptr->m_Proxy;
}

void CurrentValues::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* CurrentValues::widget() const
{
    return nullptr;
}

void CurrentValuesPrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_Proxy.setSourceModel(newModel);
}
