#include "headnode.h"

#include "proxies/headproxy.h"

#include "widgets/rowsubset.h"

class HeadNodePrivate final : public QObject
{
public:
    HeadNodePrivate(QObject* parent) : QObject(parent), m_Proxy(this) {}

    HeadProxy m_Proxy;
    RowSubset m_Widget;

    HeadNode* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

HeadNode::HeadNode(QObject* parent) : ProxyNode(parent), d_ptr(new HeadNodePrivate(this))
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &HeadNodePrivate::slotModelChanged);
    QObject::connect(&d_ptr->m_Widget, &RowSubset::maxRowChanged, &d_ptr->m_Proxy, &HeadProxy::setMaximum);
    QObject::connect(&d_ptr->m_Widget, &RowSubset::limitChanged, &d_ptr->m_Proxy, &HeadProxy::setLimited);
}

HeadNode::~HeadNode()
{
    delete d_ptr;
}

QString HeadNode::title() const
{
    return "Head filter";
}

QString HeadNode::id() const
{
    return QStringLiteral("head_node");
}

void HeadNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* HeadNode::widget() const
{
    return &d_ptr->m_Widget;
}

QAbstractItemModel* HeadNode::filteredModel() const
{
    return &d_ptr->m_Proxy;
}

void HeadNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)

    m_Proxy.setSourceModel(newModel);
}
