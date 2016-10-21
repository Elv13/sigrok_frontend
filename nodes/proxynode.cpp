#include "proxynode.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QDebug>

#include "../widgets/aquisition.h"

class ProxyNodePrivate : public QObject
{
public:
    QAbstractItemModel* m_pModel {nullptr};

};


ProxyNode::ProxyNode(QObject* parent) : AbstractNode(parent),
    d_ptr(new ProxyNodePrivate())
{
}

ProxyNode::~ProxyNode()
{
    delete d_ptr;
}

QString ProxyNode::title() const
{
    return "Aquisition";
}

QWidget* ProxyNode::widget() const
{
    return new Aquisition();
}

QAbstractItemModel* ProxyNode::model() const
{
    return d_ptr->m_pModel;
}

void ProxyNode::setModel(QAbstractItemModel* m)
{
    qDebug() << "IN SET MODEL" << m;
    auto old = d_ptr->m_pModel;
    d_ptr->m_pModel = m;
    Q_EMIT modelChanged(m, old);
}
