#include "autorangenode.h"

#include "widgets/current.h"

#include <QtCore/QDebug>

#include <QtWidgets/QScrollBar>
#include "widgets/meter.h"

#include "common/pagemanager.h"
#include "common/abstractsession.h"

#include "proxies/meterproxy.h"
#include "proxies/columnproxy.h"

#include <QDebug>
#include <QtCore/QTimer>

#include "columnserializationadapter.h"

#include <limits>

class AutoRangeNodePrivate : public QObject
{
    Q_OBJECT
public:
    QAbstractItemModel* m_pSource {nullptr};
    Meter* m_pMeterW {new Meter()};
    MeterProxy* m_pCheckProxy {new MeterProxy(this)};
    ColumnProxy* m_pColumnProxy {new ColumnProxy()};
    ColumnSerializationAdapter m_Serializer {m_pCheckProxy, {1,2}, this};

    qreal m_Min = std::numeric_limits<qreal>::max();
    qreal m_Max = std::numeric_limits<qreal>::min();

    AutoRangeNode* q_ptr;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotColumnEnabled(int col, bool enabled);
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
    void slotReset();
};

AutoRangeNode::AutoRangeNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new AutoRangeNodePrivate())
{
    d_ptr->q_ptr = this;

    d_ptr->m_pCheckProxy->setSourceModel(d_ptr->m_pColumnProxy);

    d_ptr->m_pMeterW->setModel(d_ptr->m_pCheckProxy);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &AutoRangeNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pCheckProxy, &MeterProxy::columnEnabled, d_ptr, &AutoRangeNodePrivate::slotColumnEnabled);
}

AutoRangeNode::~AutoRangeNode()
{
    delete d_ptr;
}

void AutoRangeNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    d_ptr->m_Serializer.write(parent);
}

void AutoRangeNode::read(const QJsonObject &parent)
{
    d_ptr->m_Serializer.read(parent);
}

QWidget* AutoRangeNode::widget() const
{
    return d_ptr->m_pMeterW;
}

void AutoRangeNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pSource = newModel;
    m_pColumnProxy->setSourceModel(newModel);

    if (old) {
        QObject::disconnect(old, &QAbstractItemModel::rowsInserted,
            this, &AutoRangeNodePrivate::slotRowsInserted);
        QObject::disconnect(old, &QAbstractItemModel::modelReset,
            this, &AutoRangeNodePrivate::slotReset);
        QObject::disconnect(old, &QAbstractItemModel::layoutChanged,
            this, &AutoRangeNodePrivate::slotReset);
        QObject::disconnect(old, &QAbstractItemModel::rowsRemoved,
            this, &AutoRangeNodePrivate::slotReset);
    }

    if (!newModel)
        return;

    QObject::connect(newModel, &QAbstractItemModel::rowsInserted,
        this, &AutoRangeNodePrivate::slotRowsInserted);
    QObject::connect(old, &QAbstractItemModel::modelReset,
        this, &AutoRangeNodePrivate::slotReset);
    QObject::connect(old, &QAbstractItemModel::layoutChanged,
        this, &AutoRangeNodePrivate::slotReset);
    QObject::connect(old, &QAbstractItemModel::rowsRemoved,
        this, &AutoRangeNodePrivate::slotReset);

    slotRowsInserted({}, 0, newModel->rowCount() - 1);
}

void AutoRangeNodePrivate::slotColumnEnabled(int col, bool)
{
    Q_UNUSED(col) //FIXME
}

void AutoRangeNodePrivate::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)

    const auto oldMin(m_Min), oldMax(m_Max);

    // Force a reset
    if (first == 0) {
        m_Min = std::numeric_limits<qreal>::max();
        m_Max = std::numeric_limits<qreal>::min();
    }

    for (int i=first; i <= last;i++) {
        const qreal v = m_pSource->index(i, 1).data().toReal();
        m_Min = std::min(m_Min, v);
        m_Max = std::max(m_Max, v);
    }

    if (oldMax != m_Max)
        Q_EMIT q_ptr->maximumChanged(m_Max);

    if (oldMin != m_Max)
        Q_EMIT q_ptr->minimumChanged(m_Min);
}

void AutoRangeNodePrivate::slotReset()
{
    slotRowsInserted({}, 0, m_pSource->rowCount()-1);
}

qreal AutoRangeNode::minimum() const
{
    return d_ptr->m_Min;
}

qreal AutoRangeNode::maximum() const
{
    return d_ptr->m_Max;
}

#include "autorangenode.moc"
