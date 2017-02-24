#include "scalenode.h"

#include "qwt6/qwt_thermo.h"

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

class ScaleNodePrivate : public QObject
{
    Q_OBJECT
public:
    QwtThermo* m_pCurrent {new QwtThermo(nullptr)};
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

ScaleNode::ScaleNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new ScaleNodePrivate())
{
    d_ptr->m_pCheckProxy->setSourceModel(d_ptr->m_pColumnProxy);

    QTimer::singleShot(0, [this]() {
        session()->pages()->addPage(this, d_ptr->m_pCurrent, title(), uid());
    });

    d_ptr->m_pMeterW->setModel(d_ptr->m_pCheckProxy);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &ScaleNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pCheckProxy, &MeterProxy::columnEnabled, d_ptr, &ScaleNodePrivate::slotColumnEnabled);

    d_ptr->m_pCurrent->setOriginMode( QwtThermo::OriginMode::OriginCustom );
    d_ptr->m_pCurrent->setOrigin(0.0);

}

ScaleNode::~ScaleNode()
{
    delete d_ptr;
}

void ScaleNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    d_ptr->m_Serializer.write(parent);
}

void ScaleNode::read(const QJsonObject &parent)
{
    d_ptr->m_Serializer.read(parent);
}

QWidget* ScaleNode::widget() const
{
    return d_ptr->m_pMeterW;
}

void ScaleNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pSource = newModel;
    m_pColumnProxy->setSourceModel(newModel);
//     m_pCurrent->setModel(newModel);

    if (old) {
        QObject::disconnect(old, &QAbstractItemModel::rowsInserted,
                     this, &ScaleNodePrivate::slotRowsInserted);
    }

    QObject::connect(newModel, &QAbstractItemModel::rowsInserted,
                     this, &ScaleNodePrivate::slotRowsInserted);
}

void ScaleNodePrivate::slotColumnEnabled(int col, bool)
{
    Q_UNUSED(col) //FIXME
}

void ScaleNodePrivate::slotRowsInserted()
{
    const int main = m_pCheckProxy->mainColumn();
    const auto idx = m_pSource->index(m_pSource->rowCount()-1,main);

    m_pCurrent->setValue(
        idx.data().toDouble()
    );

}

QString ScaleNode::remoteWidgetType() const
{
    return id();
}

void ScaleNode::setLowerLimit(qreal v)
{
    d_ptr->m_pCurrent->setLowerBound(v);
}

void ScaleNode::setUpperLimit(qreal v)
{
    d_ptr->m_pCurrent->setUpperBound(v);
}


#include "scalenode.moc"
