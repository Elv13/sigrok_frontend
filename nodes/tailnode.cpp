#include "tailnode.h"

#include "proxies/tailproxy.h"

#include "widgets/rowsubset.h"

class TailNodePrivate final : public QObject
{
public:
    TailNodePrivate(QObject* parent) : QObject(parent), m_Proxy(this) {}

    TailProxy m_Proxy;
    RowSubset m_Widget;

    TailNode* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

TailNode::TailNode(QObject* parent) : ProxyNode(parent), d_ptr(new TailNodePrivate(this))
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &TailNodePrivate::slotModelChanged);
    QObject::connect(&d_ptr->m_Widget, &RowSubset::maxRowChanged, &d_ptr->m_Proxy, &TailProxy::setMaximum);
    QObject::connect(&d_ptr->m_Widget, &RowSubset::limitChanged, &d_ptr->m_Proxy, &TailProxy::setLimited);
}

TailNode::~TailNode()
{
    delete d_ptr;
}

QString TailNode::title() const
{
    return "Tail filter";
}

QString TailNode::id() const
{
    return QStringLiteral("tail_node");
}

void TailNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    parent[ "count"   ] = maximumRows();
    parent[ "limited" ] = isLimited();
}

void TailNode::read(const QJsonObject &parent)
{
    setLimited    (parent["limited"].toBool());
    setMaximumRows(parent["count"  ].toInt ());
}

bool TailNode::isLimited() const
{
    return d_ptr->m_Proxy.isLimited();
}

int TailNode::maximumRows() const
{
    return d_ptr->m_Proxy.maximum();
}

void TailNode::setLimited(bool v)
{
    d_ptr->m_Proxy.setLimited(v);
}

void TailNode::setMaximumRows(int v)
{
    d_ptr->m_Proxy.setMaximum(v);
}

QWidget* TailNode::widget() const
{
    return &d_ptr->m_Widget;
}

QAbstractItemModel* TailNode::filteredModel() const
{
    return &d_ptr->m_Proxy;
}

void TailNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)

    m_Proxy.setSourceModel(newModel);
}
