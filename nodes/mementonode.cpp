#include "mementonode.h"

#include "../proxies/mementoproxy.h"

#include <QtCore/QDebug>

class MementoNodePrivate : public QObject
{
public:
    MementoProxy* m_pProxy {new MementoProxy};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

MementoNode::MementoNode(QObject* parent) : ProxyNode(parent), d_ptr(new MementoNodePrivate())
{
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &MementoNodePrivate::slotModelChanged);
}

MementoNode::~MementoNode()
{
    
}

QString MementoNode::title() const
{
    return "Memento";
}

QString MementoNode::id() const
{
    return QStringLiteral("memento_node");
}

void MementoNode::read(const QJsonObject &parent)
{
    auto proxy = new MementoProxy(parent["memento"].toObject(), this);

    d_ptr->m_pProxy = proxy;

    Q_EMIT mementoModelChanged(proxy);
}

void MementoNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
    parent["memento"] = d_ptr->m_pProxy->toJson();
}

QWidget* MementoNode::widget() const
{
    return nullptr;
}

void MementoNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pProxy->setSourceModel(newModel);
    m_pProxy->sync();
}

QAbstractItemModel* MementoNode::mementoModel() const
{
    return d_ptr->m_pProxy;
}
