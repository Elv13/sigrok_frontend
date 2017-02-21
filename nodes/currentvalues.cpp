#include "currentvalues.h"

#include <QtCore/QDebug>

#include "proxies/lastrowtolistproxy.h"

#include <kconcatenaterowsproxymodel.h>

#include "qt5-node-editor/src/qobjectmodel.h"

class AdditionalProperties : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel NOTIFY modelChanged USER true)

    QAbstractItemModel* model() const;
    void setModel(QAbstractItemModel* m);

    QAbstractItemModel* m_pModel {nullptr};

Q_SIGNALS:
    void modelChanged(QAbstractItemModel* m, QAbstractItemModel* o);
};

class CurrentValuesPrivate final : public QObject
{
    Q_OBJECT
public:
    CurrentValuesPrivate(CurrentValues* q) : QObject(q), q_ptr(q) {}

    AdditionalProperties m_Obj;
    LastRowToListProxy* m_pProxy{new LastRowToListProxy(this)};
    KConcatenateRowsProxyModel* m_pConcat{new KConcatenateRowsProxyModel(this)};

    QObjectModel* m_pObjModel {new QObjectModel {
        {&this->m_Obj},
        Qt::Vertical,
        QObjectModel::Role::PropertyNameRole,
        this
    }};

    CurrentValues* q_ptr;
public Q_SLOTS:
    void slotDataChanged();
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

CurrentValues::CurrentValues(AbstractSession* sess) : ProxyNode(sess), d_ptr(new CurrentValuesPrivate(this))
{
    d_ptr->m_pProxy->setUsingHeaderAsDisplayRole(true);
    d_ptr->m_pConcat->addSourceModel(d_ptr->m_pProxy);
    d_ptr->m_pConcat->addSourceModel(d_ptr->m_pObjModel);

    QObject::connect(&d_ptr->m_Obj, &AdditionalProperties::modelChanged, d_ptr, &CurrentValuesPrivate::slotModelChanged);
}

CurrentValues::~CurrentValues()
{
    delete d_ptr;
}

QAbstractItemModel* CurrentValues::sourceModel() const
{
    return d_ptr->m_pConcat;
}

void CurrentValues::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* CurrentValues::widget() const
{
    return nullptr;
}

void CurrentValuesPrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    Q_UNUSED(old)
    m_pProxy->setSourceModel(newModel);

    connect(newModel, &QAbstractItemModel::rowsInserted, this, &CurrentValuesPrivate::slotDataChanged);
}

QAbstractItemModel* AdditionalProperties::model() const
{
    return m_pModel;
}

void AdditionalProperties::setModel(QAbstractItemModel* m)
{
    if (m == m_pModel)
        return;

    auto o = m_pModel;
    m_pModel = m;
    Q_EMIT modelChanged(m, o);
}

void CurrentValuesPrivate::slotDataChanged()
{
    Q_EMIT m_pConcat->dataChanged(
        m_pConcat->index(0,0),m_pConcat->index(m_pConcat->rowCount()-2,0)
    );
}
#include "currentvalues.moc"
