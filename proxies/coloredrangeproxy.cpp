#include "coloredrangeproxy.h"

#include <QtCore/QDebug>

#ifdef ENABLE_KCHART
 #include <KChart/KChartGlobal.h>
 #include <KChartLineAttributes.h>
#endif

#include "columnproxy.h"
#include "rangeproxy_p.h"

#include <QtCore/QIdentityProxyModel>

class ColoredProxy;

class ColoredRangeProxyPrivate
{
public:
    ColoredProxy* m_pProxy;
    ColoredRangeProxy* q_ptr;
    bool m_Mutex {false};
};

class ColoredProxy : public QIdentityProxyModel
{
    Q_OBJECT
    friend class ColoredRangeProxy;
public:
    using QIdentityProxyModel::QIdentityProxyModel;

    virtual QVariant data(const QModelIndex& idx, int role) const override;

    ColoredRangeProxyPrivate* d_ptr;

private Q_SLOTS:
    void rangeChanged();
};

ColoredRangeProxy::ColoredRangeProxy(QObject* parent) : RangeProxy(parent),
    d_ptr(new ColoredRangeProxyPrivate())
{
    d_ptr->q_ptr = this;

    d_ptr->m_pProxy = new ColoredProxy(parent);
    d_ptr->m_pProxy->d_ptr = d_ptr;

    setExtraColumnCount(2);
    connect(this, &QAbstractItemModel::dataChanged,
        d_ptr->m_pProxy, &ColoredProxy::rangeChanged);
}

ColoredRangeProxy::~ColoredRangeProxy()
{
    delete d_ptr->m_pProxy;
    delete d_ptr;
}

QVariant ColoredRangeProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto n = static_cast<RangeProxyNode*>(idx.internalPointer());

    switch (role) {
        case Qt::UserRole+1:
            return (*n->m_hExtraValues)[Qt::BackgroundRole];
        case Qt::UserRole+2:
            return (*n->m_hExtraValues)[Qt::ForegroundRole];
        case Qt::ForegroundRole:
        case Qt::BackgroundRole:
            if (idx.column() == 1)
                return (*n->m_hExtraValues)[Qt::BackgroundRole];
            if (idx.column() == 2)
                return (*n->m_hExtraValues)[Qt::ForegroundRole];
    };

    return RangeProxy::data(idx, role);
}

void ColoredProxy::rangeChanged()
{
    //FIXME performance: Only reload the changed column
    Q_EMIT dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
}

bool ColoredRangeProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return {};

    const auto n = static_cast<RangeProxyNode*>(index.internalPointer());

    switch (role) {
        case Qt::BackgroundRole:
        case Qt::ForegroundRole:
            n->m_hExtraValues->insert(role,  value);
            Q_EMIT dataChanged(index, index);
            return true;
    };

    return RangeProxy::setData(index, value, role);
}

QVariant ColoredProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};
    // Technically, the source index could hold its own Bg/Fg roles, but then
    // it would make this proxy less useful. So it is ignored on purpose.
    switch(role) {
#ifdef ENABLE_KCHART
        case KChart::DatasetBrushRole:
#endif
        case Qt::BackgroundRole:
            return d_ptr->q_ptr->matchSourceIndex(mapToSource(idx))
                .data(Qt::UserRole+1);
#ifdef ENABLE_KCHART
        case KChart::DatasetPenRole:
#endif
        case Qt::ForegroundRole:
            return d_ptr->q_ptr->matchSourceIndex(mapToSource(idx))
                .data(Qt::UserRole+2);
#ifdef ENABLE_KCHART
        case KChart::LineAttributesRole:
            static KChart::LineAttributes attributes;
            attributes.setDisplayArea(true);
            attributes.setTransparency(127);
            return QVariant::fromValue(attributes);
#endif
    }

    return QIdentityProxyModel::data(idx, role);
}

void ColoredRangeProxy::setSourceModel(QAbstractItemModel *sourceModel)
{
    RangeProxy::setSourceModel(sourceModel);
    d_ptr->m_pProxy->setSourceModel(sourceModel);
}

QAbstractItemModel* ColoredRangeProxy::filteredModel() const
{
    return d_ptr->m_pProxy;
}

#include <coloredrangeproxy.moc>

