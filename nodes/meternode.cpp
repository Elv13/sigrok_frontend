#include "meternode.h"

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

class MeterNodePrivate : public QObject
{
    Q_OBJECT
public:
    Current* m_pCurrent {new Current(nullptr)};
    Meter* m_pMeterW {new Meter()};
    MeterProxy* m_pCheckProxy {new MeterProxy(this)};
    ColumnProxy* m_pColumnProxy {new ColumnProxy()};
    QAbstractItemModel* m_pSource {nullptr};
    ColumnSerializationAdapter m_Serializer {m_pCheckProxy, {1,2}, this};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotColumnEnabled(int col, bool enabled);
    void slotRowsInserted();
};

MeterNode::MeterNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new MeterNodePrivate())
{
    d_ptr->m_pCheckProxy->setSourceModel(d_ptr->m_pColumnProxy);

    QTimer::singleShot(0, [this]() {
        session()->pages()->addPage(this, d_ptr->m_pCurrent, title(), uid());
    });

    d_ptr->m_pMeterW->setModel(d_ptr->m_pCheckProxy);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &MeterNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pCheckProxy, &MeterProxy::columnEnabled, d_ptr, &MeterNodePrivate::slotColumnEnabled);
}

MeterNode::~MeterNode()
{
    
}

QString MeterNode::title() const
{
    return QStringLiteral("Meter");
}

QString MeterNode::id() const
{
    return QStringLiteral("meter_node");
}

void MeterNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    d_ptr->m_Serializer.write(parent);
}

void MeterNode::read(const QJsonObject &parent)
{
    d_ptr->m_Serializer.read(parent);
}

QWidget* MeterNode::widget() const
{
    return d_ptr->m_pMeterW;
}

void MeterNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pSource = newModel;
    m_pColumnProxy->setSourceModel(newModel);
    m_pCurrent->setModel(newModel);

    if (old) {
        QObject::disconnect(old, &QAbstractItemModel::rowsInserted,
                     this, &MeterNodePrivate::slotRowsInserted);
    }

    QObject::connect(newModel, &QAbstractItemModel::rowsInserted,
                     this, &MeterNodePrivate::slotRowsInserted);
}

void MeterNodePrivate::slotColumnEnabled(int col, bool)
{
    Q_UNUSED(col) //FIXME
}

void MeterNodePrivate::slotRowsInserted()
{
    const int main = m_pCheckProxy->mainColumn();
    const auto idx = m_pSource->index(m_pSource->rowCount()-1,main);

    m_pCurrent->setText(
        idx.data().toString()
    );

}

QString MeterNode::remoteWidgetType() const
{
    return id();
}

#include "meternode.moc"
