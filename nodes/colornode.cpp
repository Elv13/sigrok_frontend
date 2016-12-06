#include "colornode.h"

#include "proxies/columnproxy.h"
#include "proxies/coloredrangeproxy.h"

#include <QtWidgets/QScrollBar>
#include <KColorButton>

#include "widgets/range.h"

#include "proxies/rangeproxy.h"

#include <QtCore/QDebug>
#include <QtCore/QJsonArray>

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

    QJsonArray columns;

    for (int i = 0; i< d_ptr->m_pRangeProxy->rowCount(); i++) {
        const auto idx = d_ptr->m_pRangeProxy->index(i, 0);

        if (d_ptr->m_pRangeProxy->rowCount(idx)) {
            QJsonObject col;
            const QString column = idx.data().toString();

            col[ "name"      ] = column;
            col[ "bg"        ] = d_ptr->m_pRangeProxy->index(0, 1, idx)
                .data(Qt::BackgroundRole).toString();
            col[ "fg"        ] = d_ptr->m_pRangeProxy->index(0, 1, idx)
                .data(Qt::ForegroundRole).toString();
            col[ "delimiter" ] = d_ptr->m_pRangeProxy->index(0, 0, idx)
                .data((int)RangeProxy::Role::RANGE_DELIMITER_NAME).toString();

            Q_ASSERT(d_ptr->m_pRangeProxy->index(0, 0, idx).parent() == idx);
            Q_ASSERT(d_ptr->m_pRangeProxy->index(0, 1, idx).parent() == idx);
            Q_ASSERT(d_ptr->m_pRangeProxy->index(0, 2, idx).parent() == idx);

            columns.append(col);
        }
    }

    parent["columns"] = columns;
}

void ColorNode::read(const QJsonObject &parent)
{
    qDebug() << "\n\n\nIN READ COLOR";
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
    Q_UNUSED(old)
    m_pRangeProxy->setSourceModel(newModel);
}
/*
void ColorNodePrivate::slotDataChanged()
{
}*/
