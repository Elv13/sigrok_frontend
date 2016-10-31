#include "coloredrangeproxy.h"

#include <QtCore/QDebug>

#include <KChart/KChartGlobal.h>
#include <KChartLineAttributes.h>

#include "../proxies/columnproxy.h"

#include <QtCore/QIdentityProxyModel>

class ColoredProxy;

class ColoredRangeProxyPrivate
{
public:
    QVector< QVector<QVariant> > m_llColors {{}, {}};
//     ColumnProxy* m_pColumnProxy {new ColumnProxy()};
    ColoredProxy* m_pProxy;
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
//     RangeProxy::setSourceModel();

    d_ptr->m_pProxy = new ColoredProxy(parent);
    d_ptr->m_pProxy->d_ptr = d_ptr;

    setExtraColumnCount(2);
}

ColoredRangeProxy::~ColoredRangeProxy()
{
    delete d_ptr->m_pProxy;
    delete d_ptr;
}

void ColoredRangeProxy::setBackgroundColor(int row, const QVariant& color)
{
    if (row >= d_ptr->m_llColors.size())
        d_ptr->m_llColors.resize(row+1);

    d_ptr->m_llColors[row].resize(2);

    d_ptr->m_llColors[row][0] = color;
}

void ColoredRangeProxy::setForegroundColor(int row, const QVariant& color)
{
    if (row >= d_ptr->m_llColors.size())
        d_ptr->m_llColors.resize(row+1);

    d_ptr->m_llColors[row].resize(2);

    d_ptr->m_llColors[row][1] = color;
}

QVariant ColoredProxy::data(const QModelIndex& idx, int role) const
{
    if (d_ptr->m_llColors.size() > idx.column()) {
        switch(role) {
            case KChart::DatasetBrushRole:
            case Qt::BackgroundRole:
                return d_ptr->m_llColors[idx.column()][0];
                break;
            case KChart::DatasetPenRole:
            case Qt::ForegroundRole:
                return d_ptr->m_llColors[idx.column()][1];
                break;
            case KChart::LineAttributesRole:
                static KChart::LineAttributes attributes;
                attributes.setDisplayArea(true);
                attributes.setTransparency(127);
                return QVariant::fromValue(attributes);
        }
    }

    return QIdentityProxyModel::data(idx, role);
}

void ColoredRangeProxy::setSourceModel(QAbstractItemModel *sourceModel)
{
//     d_ptr->m_pColumnProxy->setSourceModel(sourceModel);

    RangeProxy::setSourceModel(sourceModel);

    d_ptr->m_pProxy->setSourceModel(sourceModel);
}

QAbstractItemModel* ColoredRangeProxy::filteredModel() const
{
    return d_ptr->m_pProxy;
}
