#include "coloredrangeproxy.h"

#include <QtCore/QDebug>

#include <KChart/KChartGlobal.h>
#include <KChartLineAttributes.h>

#include "columnproxy.h"
#include "rangeproxy_p.h"

#include <QtCore/QIdentityProxyModel>

class ColoredProxy;

class ColoredRangeProxyPrivate
{
public:
    ColoredProxy* m_pProxy;
    ColoredRangeProxy* q_ptr;
};

class ColoredProxy : public QIdentityProxyModel
{
public:
    using QIdentityProxyModel::QIdentityProxyModel;

    virtual QVariant data(const QModelIndex& idx, int role) const override;

    ColoredRangeProxyPrivate* d_ptr;
};

ColoredRangeProxy::ColoredRangeProxy(QObject* parent) : RangeProxy(parent),
    d_ptr(new ColoredRangeProxyPrivate())
{
    d_ptr->q_ptr = this;

    d_ptr->m_pProxy = new ColoredProxy(parent);
    d_ptr->m_pProxy->d_ptr = d_ptr;

    setExtraColumnCount(2);
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

    const auto n = static_cast<Node*>(idx.internalPointer());

    switch (role) {
        case Qt::BackgroundRole:
        case Qt::ForegroundRole:
            return (*n->m_hExtraValues)[role];
    };

    return RangeProxy::data(idx, role);
}

bool ColoredRangeProxy::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return {};

    const auto n = static_cast<Node*>(index.internalPointer());

    switch (role) {
        case Qt::BackgroundRole:
        case Qt::ForegroundRole:
            (*n->m_hExtraValues)[role] = value;
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
        case KChart::DatasetBrushRole:
        case Qt::BackgroundRole:
            return d_ptr->q_ptr->matchSourceIndex(mapToSource(idx))
                .data(Qt::BackgroundRole);
        case KChart::DatasetPenRole:
        case Qt::ForegroundRole:
            return d_ptr->q_ptr->matchSourceIndex(mapToSource(idx))
                .data(Qt::ForegroundRole);
        case KChart::LineAttributesRole:
            static KChart::LineAttributes attributes;
            attributes.setDisplayArea(true);
            attributes.setTransparency(127);
            return QVariant::fromValue(attributes);
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
