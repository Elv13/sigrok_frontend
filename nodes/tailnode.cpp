#include "tailnode.h"

#include "proxies/tailproxy.h"

#include "widgets/rowsubset.h"

class TailNodePrivate final : public QObject
{
    Q_OBJECT
public:
    TailNodePrivate(QObject* parent) : QObject(parent), m_Proxy(this) {}

    TailProxy m_Proxy;
    RowSubset* m_pWidget {nullptr};

    TailNode* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

TailNode::TailNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new TailNodePrivate(this))
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &TailNodePrivate::slotModelChanged);
}

TailNode::~TailNode()
{
    delete d_ptr;
}

QString TailNode::title() const
{
    return QStringLiteral("Tail filter");
}

QString TailNode::id() const
{
    return QStringLiteral("tail_node");
}

void TailNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    parent[ QStringLiteral("count")   ] = maximumRows();
    parent[ QStringLiteral("limited") ] = isLimited();
}

void TailNode::read(const QJsonObject &parent)
{
    setLimited    (parent[QStringLiteral("limited")].toBool());
    setMaximumRows(parent[QStringLiteral("count")  ].toInt ());
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
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new RowSubset();
        QObject::connect(d_ptr->m_pWidget, &RowSubset::maxRowChanged, &d_ptr->m_Proxy, &TailProxy::setMaximum);
        QObject::connect(d_ptr->m_pWidget, &RowSubset::limitChanged, &d_ptr->m_Proxy, &TailProxy::setLimited);
    }
    return d_ptr->m_pWidget;
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

QStringList TailNode::searchTags() const
{
    static QStringList l {
        QStringLiteral("slice"),
        QStringLiteral("frame"),
        QStringLiteral("wave"),
        QStringLiteral("still"),
        QStringLiteral("window"),
    };

    return l;
}

#include <tailnode.moc>
