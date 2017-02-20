#include "remotemeter.h"

#include "meterdata_source.h"
#include "../widgets/meter.h"

#include <QRemoteObjectHost>
#include "common/remotemanager.h"


class MeterData final : public MeterDataSimpleSource
{
    Q_OBJECT
public:
    explicit MeterData(QObject* parent = nullptr) : MeterDataSimpleSource(parent) {}

private:
    //float m_MainValue {0};
};

class RemoteMeterPrivate final : public QObject
{
    Q_OBJECT
public:

    // Remote objects
    MeterData m_MeterData;

    RemoteMeter* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotRowsInserted();
};

RemoteMeter::RemoteMeter(AbstractSession* sess) : ProxyNode(sess), d_ptr(new RemoteMeterPrivate())
{
    d_ptr->q_ptr = this;

    RemoteManager::instance()->addObject(&d_ptr->m_MeterData, QStringLiteral("Remote_meter"));

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &RemoteMeterPrivate::slotModelChanged);
}

RemoteMeter::~RemoteMeter()
{
    delete d_ptr;
}

void RemoteMeter::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* RemoteMeter::widget() const
{
    return nullptr;
}

void RemoteMeterPrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    if (old) {
        QObject::disconnect(old, &QAbstractItemModel::rowsInserted,
                     this, &RemoteMeterPrivate::slotRowsInserted);
    }

    QObject::connect(newModel, &QAbstractItemModel::rowsInserted,
                     this, &RemoteMeterPrivate::slotRowsInserted);
}

void RemoteMeterPrivate::slotRowsInserted()
{
//     const int main = m_pCheckProxy->mainColumn();
    const auto idx = q_ptr->model()->index(q_ptr->model()->rowCount()-1,0);

    m_MeterData.setMainValue(idx.data().toFloat());
}

#include "remotemeter.moc"
