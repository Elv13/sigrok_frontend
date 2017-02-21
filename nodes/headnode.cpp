#include "headnode.h"

#include "proxies/headproxy.h"

#include "widgets/rowsubset.h"

class HeadNodePrivate final : public QObject
{
    Q_OBJECT
public:
    HeadNodePrivate(QObject* parent) : QObject(parent), m_Proxy(this) {}

    HeadProxy m_Proxy;
    RowSubset* m_pWidget {nullptr};

    HeadNode* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

HeadNode::HeadNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new HeadNodePrivate(this))
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &HeadNodePrivate::slotModelChanged);
}

HeadNode::~HeadNode()
{
    delete d_ptr;
}

void HeadNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    parent[ QStringLiteral("count")   ] = maximumRows();
    parent[ QStringLiteral("limited") ] = isLimited();
}

void HeadNode::read(const QJsonObject &parent)
{
    setLimited    (parent[QStringLiteral("limited")].toBool());
    setMaximumRows(parent[QStringLiteral("count")  ].toInt ());
}

bool HeadNode::isLimited() const
{
    return d_ptr->m_Proxy.isLimited();
}

int HeadNode::maximumRows() const
{
    return d_ptr->m_Proxy.maximum();
}

void HeadNode::setLimited(bool v)
{
    d_ptr->m_Proxy.setLimited(v);
}

void HeadNode::setMaximumRows(int v)
{
    d_ptr->m_Proxy.setMaximum(v);
}

QWidget* HeadNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new RowSubset();
        QObject::connect(d_ptr->m_pWidget, &RowSubset::maxRowChanged, &d_ptr->m_Proxy, &HeadProxy::setMaximum);
        QObject::connect(d_ptr->m_pWidget, &RowSubset::limitChanged, &d_ptr->m_Proxy, &HeadProxy::setLimited);
    }
    return d_ptr->m_pWidget;
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

#include <headnode.moc>
