#include "meternode.h"

#include "../widgets/current.h"

#include <QtCore/QDebug>

#include <QtWidgets/QScrollBar>
#include "../widgets/meter.h"

#include "../mainwindow.h"

#include "../proxies/meterproxy.h"
#include "../proxies/columnproxy.h"

#include <QDebug>

#include "meterdata_source.h"

#include <QRemoteObjectHost>

class MeterData final : public MeterDataSimpleSource
{
    Q_OBJECT
public:
    explicit MeterData(QObject* parent = nullptr) : MeterDataSimpleSource(parent) {}

private:
    float m_MainValue {0};
};

class MeterNodePrivate : public QObject
{
    Q_OBJECT
public:
    Current* m_pCurrent {new Current(nullptr)};
    Meter* m_pMeterW {new Meter()};
    MeterProxy* m_pCheckProxy {new MeterProxy(this)};
    ColumnProxy* m_pColumnProxy {new ColumnProxy()};
    QAbstractItemModel* m_pSource {nullptr};

    // Remote objects
//     MeterData m_MeterData;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotColumnEnabled(int col, bool enabled);
    void slotRowsInserted();
};

MeterNode::MeterNode(QObject* parent) : ProxyNode(parent), d_ptr(new MeterNodePrivate())
{
    d_ptr->m_pCheckProxy->setSourceModel(d_ptr->m_pColumnProxy);

    MainWindow::addDock(d_ptr->m_pCurrent, "Meter");

    d_ptr->m_pMeterW->m_pContent->setModel(d_ptr->m_pCheckProxy);
    d_ptr->m_pMeterW->m_pContent->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    d_ptr->m_pMeterW->m_pContent->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    d_ptr->m_pMeterW->m_pContent->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &MeterNodePrivate::slotModelChanged);
    QObject::connect(d_ptr->m_pCheckProxy, &MeterProxy::columnEnabled, d_ptr, &MeterNodePrivate::slotColumnEnabled);

//     auto srcNode = new QRemoteObjectHost(QUrl(QStringLiteral("tcp://10.10.10.136:2223")),QUrl(QStringLiteral("tcp://10.10.10.136:2223")));
//     srcNode->enableRemoting(&d_ptr->m_MeterData, QStringLiteral("RemoteMeterData"));
}

MeterNode::~MeterNode()
{
    
}

QString MeterNode::title() const
{
    return "Meter";
}

QString MeterNode::id() const
{
    return QStringLiteral("meter_node");
}

void MeterNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
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
    
}

void MeterNodePrivate::slotRowsInserted()
{
    const int main = m_pCheckProxy->mainColumn();
    const auto idx = m_pSource->index(m_pSource->rowCount()-1,main);

    m_pCurrent->m_pValue->setText(
        idx.data().toString()
    );

//     m_MeterData.setMainValue(idx.data().toFloat());
}

#include "meternode.moc"
