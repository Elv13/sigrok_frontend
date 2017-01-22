#include "lcdmeternode.h"

#include "widgets/lcdmeter.h"

#include <QtCore/QDebug>
#include <QtCore/QIdentityProxyModel>

#include <QtWidgets/QScrollBar>
#include "widgets/meter.h"

#include "common/pagemanager.h"

#include "remotemanager.h"

#include "proxies/meterproxy.h"
#include "proxies/columnproxy.h"
#include "columnserializationadapter.h"

#include <QtCore/QTimer>

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
    mutable QIdentityProxyModel* m_pRemoteModel {nullptr};
    mutable QString m_Id;
    ColumnSerializationAdapter m_Serializer {m_pCheckProxy, {1,2}, this};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotColumnEnabled(int col, bool enabled);
//     void slotRowsInserted();
};

LCDMeterNode::LCDMeterNode(QObject* parent) : ProxyNode(parent), d_ptr(new LCDMeterNodePrivate())
{
    d_ptr->m_pCheckProxy->setSourceModel(d_ptr->m_pColumnProxy);

    QTimer::singleShot(0, [this]() {
        PageManager::instance()->addPage(this, &d_ptr->m_Current, title(), uid());
    });

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

    if (m_pRemoteModel)
        m_pRemoteModel->setSourceModel(newModel);
}

void LCDMeterNodePrivate::slotColumnEnabled(int col, bool)
{
    Q_UNUSED(col) //FIXME
}

QString LCDMeterNode::remoteModelName() const
{
    if (!d_ptr->m_pRemoteModel) {
        static int count = 1;
        d_ptr->m_Id = id()+QString::number(count++);

        d_ptr->m_pRemoteModel = new QIdentityProxyModel(const_cast<LCDMeterNode*>(this));
        d_ptr->m_pRemoteModel->setSourceModel(model());

        RemoteManager::instance()->addModel(d_ptr->m_pRemoteModel, {
            Qt::DisplayRole,
            Qt::EditRole,
        }, d_ptr->m_Id);
    }

    return d_ptr->m_Id;
}

QString LCDMeterNode::remoteWidgetType() const
{
    return id();
}

#include "lcdmeternode.moc"
