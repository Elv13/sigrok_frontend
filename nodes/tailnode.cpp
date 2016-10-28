#include "tailnode.h"

#include "../proxies/tailproxy.h"

#include "../widgets/rowsubset.h"

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
    QObject::connect(d_ptr->m_Widget.spinBox, SIGNAL(valueChanged(int)), &d_ptr->m_Proxy, SLOT(setMaximum(int)));
    QObject::connect(d_ptr->m_Widget.checkBox, &QCheckBox::toggled, &d_ptr->m_Proxy, &TailProxy::setLimited);
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
