#include "currentvalues.h"

#include "proxies/lastrowtolistproxy.h"

#include <kconcatenaterowsproxymodel.h>

#include "qt5-node-editor/src/qobjectmodel.h"

class AdditionalProperties : public QObject
{
    Q_OBJECT
public:
    Q_PROPERTY(QAbstractItemModel* model READ model WRITE setModel NOTIFY modelChanged USER true);

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
    LastRowToListProxy m_Proxy{this};
    KConcatenateRowsProxyModel m_Concat{this};

    QObjectModel m_ObjModel{
        {&this->m_Obj},
        Qt::Vertical,
        QObjectModel::Role::PropertyNameRole,
        this
    };

    CurrentValues* q_ptr;
public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

CurrentValues::CurrentValues(QObject* parent) : ProxyNode(parent), d_ptr(new CurrentValuesPrivate(this))
{
    d_ptr->m_Proxy.setUsingHeaderAsDisplayRole(true);
    d_ptr->m_Concat.addSourceModel(&d_ptr->m_ObjModel);
    d_ptr->m_Concat.addSourceModel(&d_ptr->m_Proxy);


    QObject::connect(&d_ptr->m_Obj, &AdditionalProperties::modelChanged, d_ptr, &CurrentValuesPrivate::slotModelChanged);
}

CurrentValues::~CurrentValues()
{
    delete d_ptr;
}

QString CurrentValues::title() const
{
    return "Current Values";
}

QString CurrentValues::id() const
{
    return QStringLiteral("currentvalues_node");
}

QAbstractItemModel* CurrentValues::sourceModel() const
{
    return &d_ptr->m_Concat;
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
    m_Proxy.setSourceModel(newModel);
}

QAbstractItemModel* AdditionalProperties::model() const
{
    return m_pModel;
}

void AdditionalProperties::setModel(QAbstractItemModel* m)
{
    auto o = m_pModel;
    m_pModel = m;
    Q_EMIT modelChanged(m, o);
}

#include "currentvalues.moc"
