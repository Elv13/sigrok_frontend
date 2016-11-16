#include "colornode.h"

#include "../proxies/columnproxy.h"
#include "../proxies/coloredrangeproxy.h"

#include <QtWidgets/QScrollBar>
#include <KColorButton>

#include "../widgets/range.h"

#include <QDebug>

#include "../mainwindow.h"

class ColorNodePrivate : public QObject
{
public:
    ColorNodePrivate(ColorNode* p) : QObject(p) {}

    Range m_Widget;

    ColoredRangeProxy* m_pRangeProxy {new ColoredRangeProxy(this)};


public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
    void slotDataChanged();
};

ColorNode::ColorNode(QObject* parent) : ProxyNode(parent), d_ptr(new ColorNodePrivate(this))
{
    d_ptr->m_Widget.setRangeProxy(d_ptr->m_pRangeProxy);

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &ColorNodePrivate::slotModelChanged);
//     QObject::connect(d_ptr->m_pCheckable, &QAbstractItemModel::dataChanged, d_ptr, &ColorNodePrivate::slotDataChanged);

    d_ptr->m_Widget.setColumnWidgetFactory(1, [this](const QPersistentModelIndex& idx) -> QWidget* {
        auto w = new KColorButton();
        w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        QObject::connect(w, &KColorButton::changed, [this, idx](const QColor& col) {
//             d_ptr->m_pRangeProxy->setBackgroundColor(row, col);
            d_ptr->m_pRangeProxy->setData(idx, col, Qt::BackgroundRole);
        });
        return w;
    });
    d_ptr->m_Widget.setColumnWidgetFactory(2, [this](const QPersistentModelIndex& idx) -> QWidget* {
        auto w = new KColorButton();
        w->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
        QObject::connect(w, &KColorButton::changed, [this, idx](const QColor& col) {
            d_ptr->m_pRangeProxy->setData(idx, col, Qt::ForegroundRole);
//             d_ptr->m_pRangeProxy->setForegroundColor(row, col);
        });
        return w;
    });
}

ColorNode::~ColorNode()
{
    delete d_ptr;
}

QString ColorNode::title() const
{
    return "Color";
}

QString ColorNode::id() const
{
    return QStringLiteral("color_node");
}

void ColorNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QWidget* ColorNode::widget() const
{
    return &d_ptr->m_Widget;
}

QAbstractItemModel* ColorNode::filteredModel() const
{
    return d_ptr->m_pRangeProxy->filteredModel();
}

void ColorNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pRangeProxy->setSourceModel(newModel);
}
/*
void ColorNodePrivate::slotDataChanged()
{
}*/
