#include "deduplicatenode.h"

#include "proxies/columnproxy.h"

#include <QtWidgets/QScrollBar>

#include "widgets/deduplicate.h"

#include <QtCore/QItemSelectionModel>

#include "proxies/deduplicateproxy.h"

#include <KCheckableProxyModel>

#include <QtCore/QDebug>

class DeduplicateNodePrivate : public QObject
{
public:
    Deduplicate* m_pDeduplicate{Q_NULLPTR};
    QString m_PreferredColumn;

    DeduplicateProxy* m_pFilteredModel {
        new DeduplicateProxy(this)
    };

    DeduplicateNode* q_ptr;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

DeduplicateNode::DeduplicateNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new DeduplicateNodePrivate())
{
    d_ptr->q_ptr = this;

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &DeduplicateNodePrivate::slotModelChanged);

    d_ptr->m_pFilteredModel->setExtraColumnName("Samples");

}

DeduplicateNode::~DeduplicateNode()
{
    delete d_ptr;
}

QString DeduplicateNode::title() const
{
    return "Deduplicate";
}

void DeduplicateNode::read(const QJsonObject &parent)
{
    AbstractNode::read(parent);

    d_ptr->m_pFilteredModel->setExtraColumn(parent["add_column"].toBool());
    d_ptr->m_pFilteredModel->setThreshold(parent["threshold" ].toDouble());
    d_ptr->m_PreferredColumn = parent["column"].toString();
}

void DeduplicateNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    parent["add_column"] = hasExtraColumn();
    parent["threshold" ] = threshold();

    if (d_ptr->m_pDeduplicate)
        parent["column"] = d_ptr->m_pDeduplicate->selectedColumnName();
}

bool DeduplicateNode::hasExtraColumn() const
{
    return d_ptr->m_pFilteredModel->hasExtraColumn();
}

qreal DeduplicateNode::threshold() const
{
    return d_ptr->m_pFilteredModel->threshold().toDouble();
}

void DeduplicateNode::setThreshold(qreal v)
{
    if (v == d_ptr->m_pFilteredModel->threshold())
        return;

    d_ptr->m_pFilteredModel->setThreshold(v);

    Q_ASSERT(v == d_ptr->m_pFilteredModel->threshold());

    Q_EMIT thresholdChanged(v);
}


QString DeduplicateNode::id() const
{
    return QStringLiteral("deduplicate_node");
}

QWidget* DeduplicateNode::widget() const
{
    if (!d_ptr->m_pDeduplicate) {
        d_ptr->m_pDeduplicate = new Deduplicate(this);
        d_ptr->m_pDeduplicate->setPreferredColumn(d_ptr->m_PreferredColumn);

        connect(d_ptr->m_pDeduplicate, &Deduplicate::currentColumnChanged , d_ptr->m_pFilteredModel, &DeduplicateProxy::setFilterKeyColumn  );

        if (model())
            d_ptr->m_pDeduplicate->setModel(model());

        connect(d_ptr->m_pDeduplicate, &Deduplicate::enableExtraColumn, d_ptr->m_pFilteredModel, &DeduplicateProxy::setExtraColumn);
        connect(d_ptr->m_pDeduplicate, &Deduplicate::thresholdChanged , d_ptr->m_pFilteredModel, &DeduplicateProxy::setThreshold  );
    }

    return d_ptr->m_pDeduplicate;
}

QAbstractItemModel* DeduplicateNode::filteredModel() const
{
    return d_ptr->m_pFilteredModel;
}

void DeduplicateNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)

    if (m_pDeduplicate)
        m_pDeduplicate->setModel(newModel);

    m_pFilteredModel->setSourceModel(newModel);
}
