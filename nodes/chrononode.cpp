#include "chrononode.h"

#include <QtCore/QDebug>

#include "proxies/chronometerproxy.h"

#include "widgets/chrono.h"

#include "sigrokd/aquisitionmodel.h"

class ChronoNodePrivate final : public QObject
{
    Q_OBJECT
public:
    ChronometerProxy m_Proxy{this};
    ChronoNode* q_ptr;
    Chrono* m_pWidget {nullptr};
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

ChronoNode::ChronoNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new ChronoNodePrivate())
{
    d_ptr->q_ptr = this;
    d_ptr->m_Proxy.setTimeSourceColumn(0);
    d_ptr->m_Proxy.setTimeSourceRole((int)AquisitionModel::Role::U_TIMESTAMP);
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

    parent["columns"] = d_ptr->m_Proxy.extraColumns();
}

void ChronoNode::read(const QJsonObject &parent)
{
    Q_UNUSED(parent)

    d_ptr->m_Proxy.setExtraColumns( (ChronometerProxy::ExtraColumns) (
        parent["columns"].toInt() | ChronometerProxy::ExtraColumns::MS_DELTA
    ));
}

QWidget* ChronoNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Chrono(const_cast<ChronoNode*>(this));
    }

    return d_ptr->m_pWidget;
}

void ChronoNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_Proxy.setSourceModel(newModel);
}


void ChronoNode::enableDelta(bool v)
{
    int cols = d_ptr->m_Proxy.extraColumns();
    cols = v ?
        ( ( cols) | ChronometerProxy::ExtraColumns::MS_DELTA):
        ( cols & (~ChronometerProxy::ExtraColumns::MS_DELTA));

    d_ptr->m_Proxy.setExtraColumns((ChronometerProxy::ExtraColumns)cols);
}

void ChronoNode::enableMsEpoch(bool v)
{
    int cols = d_ptr->m_Proxy.extraColumns();
    cols = v ?
        ( ( cols) | ChronometerProxy::ExtraColumns::MS_EPOCH):
        ( cols & (~ChronometerProxy::ExtraColumns::MS_EPOCH));

    d_ptr->m_Proxy.setExtraColumns((ChronometerProxy::ExtraColumns)cols);
}

void ChronoNode::enableTimestamp(bool v)
{
    int cols = d_ptr->m_Proxy.extraColumns();
    cols = v ?
        ( ( cols) | ChronometerProxy::ExtraColumns::TIMESTAMP):
        ( cols & (~ChronometerProxy::ExtraColumns::TIMESTAMP));

    d_ptr->m_Proxy.setExtraColumns((ChronometerProxy::ExtraColumns)cols);
}

void ChronoNode::enableDateTime(bool v)
{
    int cols = d_ptr->m_Proxy.extraColumns();
    cols = v ?
        ( ( cols) | ChronometerProxy::ExtraColumns::DATE_TIME):
        ( cols & (~ChronometerProxy::ExtraColumns::DATE_TIME));

    d_ptr->m_Proxy.setExtraColumns((ChronometerProxy::ExtraColumns)cols);
}

bool ChronoNode::hasDeltaColumn() const
{
    return d_ptr->m_Proxy.extraColumns() & ChronometerProxy::ExtraColumns::MS_DELTA;
}

bool ChronoNode::hasMsEpochColumn() const
{
    return d_ptr->m_Proxy.extraColumns() & ChronometerProxy::ExtraColumns::MS_EPOCH;
}

bool ChronoNode::hasTimestampColumn() const
{
    return d_ptr->m_Proxy.extraColumns() & ChronometerProxy::ExtraColumns::TIMESTAMP;
}

bool ChronoNode::hasDateTimeColumn() const
{
    return d_ptr->m_Proxy.extraColumns() & ChronometerProxy::ExtraColumns::DATE_TIME;
}

#include <chrononode.moc>
