#include "coloredrangeproxy.h"

#include <KColorButton>

#include <QtWidgets/QWidget>

#include <QtGui/QColor>
#include <KChart/KChartGlobal.h>
#include <KChartLineAttributes.h>

#include "../proxies/columnproxy.h"

class ColoredProxy;

class ColoredRangeProxyPrivate
{
public:
    QVector< QVector<QColor> > m_llColors {{}, {}};
    ColumnProxy* m_pColumnProxy {new ColumnProxy()};
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
    RangeProxy::setSourceModel(d_ptr->m_pColumnProxy);

    d_ptr->m_pProxy = new ColoredProxy(parent);
    d_ptr->m_pProxy->d_ptr = d_ptr;

    setExtraColumnCount(2);
    setColumnWidgetFactory(1, [this](int row) -> QWidget* {
        auto w = new KColorButton();
        QObject::connect(w, &KColorButton::changed, [this, row](const QColor& col) {
            d_ptr->m_llColors[0].resize(columnCount());
            d_ptr->m_llColors[1].resize(columnCount());
            d_ptr->m_llColors[0][row] = col;
        });
        return w;
    });
    setColumnWidgetFactory(2, [this](int row) -> QWidget* {
        auto w = new KColorButton();
        QObject::connect(w, &KColorButton::changed, [this, row](const QColor& col) {
            d_ptr->m_llColors[0].resize(columnCount());
            d_ptr->m_llColors[1].resize(columnCount());
            d_ptr->m_llColors[1][row] = col;
        });
        return w;
    });
}

ColoredRangeProxy::~ColoredRangeProxy()
{
    delete d_ptr->m_pProxy;
    delete d_ptr;
}

QVariant ColoredProxy::data(const QModelIndex& idx, int role) const
{
    if (idx.column() && d_ptr->m_llColors.size() > 1
      && d_ptr->m_llColors[0].size() > idx.column()) {
        switch(role) {
            case KChart::DatasetBrushRole:
            case Qt::BackgroundRole:
                return d_ptr->m_llColors[0][idx.column()];
                break;
            case KChart::DatasetPenRole:
            case Qt::ForegroundRole:
                return d_ptr->m_llColors[1][idx.column()];
                break;
            case KChart::LineAttributesRole:
                static KChart::LineAttributes attributes;
                attributes.setDisplayArea(true);
//                 attributes.setTransparency(127);
                return QVariant::fromValue(attributes);
        }
    }

    return QIdentityProxyModel::data(idx, role);
}

void ColoredRangeProxy::setSourceModel(QAbstractItemModel *sourceModel)
{
    d_ptr->m_pColumnProxy->setSourceModel(sourceModel);
    d_ptr->m_pProxy->setSourceModel(sourceModel);
}

QAbstractItemModel* ColoredRangeProxy::filteredModel() const
{
    return d_ptr->m_pProxy;
}
