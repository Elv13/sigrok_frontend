#include "lcdmeternode.h"

#include "widgets/lcdmeter.h"

#include <QtCore/QDebug>

#include <QtWidgets/QScrollBar>
#include "widgets/meter.h"

#include "common/pagemanager.h"

#include "proxies/meterproxy.h"
#include "proxies/columnproxy.h"
#include "columnserializationadapter.h"

#include <QDebug>

class LCDMeterNodePrivate : public QObject
{
    Q_OBJECT
public:
    LCDMeter m_Current;
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

LCDMeterNode::LCDMeterNode(QObject* parent) : ProxyNode(parent), d_ptr(new LCDMeterNodePrivate())
{
    d_ptr->m_pCheckProxy->setSourceModel(d_ptr->m_pColumnProxy);

    PageManager::instance()->addPage(&d_ptr->m_Current, "Meter");

    d_ptr->m_pMeterW->setModel(d_ptr->m_pCheckProxy);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &LCDMeterNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pCheckProxy, &MeterProxy::columnEnabled, d_ptr, &LCDMeterNodePrivate::slotColumnEnabled);
}

LCDMeterNode::~LCDMeterNode()
{
    
}

QString LCDMeterNode::title() const
{
    return "LCD Meter";
}

QString LCDMeterNode::id() const
{
    return QStringLiteral("lcdmeter_node");
}

void LCDMeterNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    d_ptr->m_Serializer.write(parent);
}

void LCDMeterNode::read(const QJsonObject &parent)
{
    d_ptr->m_Serializer.read(parent);
}

QWidget* LCDMeterNode::widget() const
{
    return d_ptr->m_pMeterW;
}

void LCDMeterNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pSource = newModel;
    m_pColumnProxy->setSourceModel(newModel);
    m_Current.setModel(newModel);

    if (old) {
        QObject::disconnect(old, &QAbstractItemModel::rowsInserted,
                     this, &LCDMeterNodePrivate::slotRowsInserted);
    }

    QObject::connect(newModel, &QAbstractItemModel::rowsInserted,
                     this, &LCDMeterNodePrivate::slotRowsInserted);
}

void LCDMeterNodePrivate::slotColumnEnabled(int col, bool)
{
    Q_UNUSED(col) //FIXME
}

void LCDMeterNodePrivate::slotRowsInserted()
{
    const int main = m_pCheckProxy->mainColumn();
    const auto idx = m_pSource->index(m_pSource->rowCount()-1,main);

    m_Current.setValue(
        idx.data().toFloat()
    );

}

#include "lcdmeternode.moc"
