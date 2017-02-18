#include "proxynode.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDebug>


class ProxyNodePrivate : public QObject
{
    Q_OBJECT
public:
    QAbstractItemModel* m_pModel {nullptr};

};


ProxyNode::ProxyNode(AbstractSession* sess) : AbstractNode(sess),
    d_ptr(new ProxyNodePrivate())
{
}

ProxyNode::~ProxyNode()
{
    delete d_ptr;
}

QString ProxyNode::title() const
{
    return QStringLiteral("Proxy node");
}

QString ProxyNode::id() const
{
    return QStringLiteral("proxy_node");
}

QWidget* ProxyNode::widget() const
{
    return nullptr;
}

QAbstractItemModel* ProxyNode::model() const
{
    return d_ptr->m_pModel;
}

void ProxyNode::setModel(QAbstractItemModel* m)
{
    auto old = d_ptr->m_pModel;

    if (m == old)
        return;

    d_ptr->m_pModel = m;
    Q_EMIT modelChanged(m, old);
}

#include <proxynode.moc>
