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
