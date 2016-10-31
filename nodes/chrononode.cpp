#include "chrononode.h"

#include "../proxies/chronometerproxy.h"

class ChronoNodePrivate final : public QObject
{
public:
    ChronometerProxy m_Proxy{this};
    ChronoNode* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

ChronoNode::ChronoNode(QObject* parent) : ProxyNode(parent), d_ptr(new ChronoNodePrivate())
{
    d_ptr->q_ptr = this;
    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &ChronoNodePrivate::slotModelChanged);
}

ChronoNode::~ChronoNode()
{
    delete d_ptr;
}


QAbstractItemModel* ChronoNode::filteredModel() const
{
    return &d_ptr->m_Proxy;
}

QString ChronoNode::title() const
{
    return "Chronometer";
}

QString ChronoNode::id() const
{
    return QStringLiteral("chrono_node");
}

void ChronoNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* ChronoNode::widget() const
{
    return nullptr;
}

void ChronoNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_Proxy.setSourceModel(newModel);
}
