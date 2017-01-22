/* See LICENSE file for copyright and license details. */

#include "graphicsnode.hpp"
#include <QtCore/QDebug>
#include <QPen>
#include <QPainter>
#include <QTextCursor>
#include <QPainterPath>
#include <QGraphicsProxyWidget>
#include <QGraphicsPixmapItem>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsTextItem>
#include <QGraphicsDropShadowEffect>
#include <QtWidgets/QWidget>
#include <QtGui/QFontMetrics>
#include <QtGui/QIcon>

#include <algorithm>

#include "graphicsnode_p.h"

#include "graphicsbezieredge.hpp"
#include "graphicsnodesocket.hpp"
#include "graphicsnodesocket_p.h"

#include "qnodeeditorsocketmodel.h"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class CloseButton : public QGraphicsTextItem
{
public:
    explicit CloseButton(NodeGraphicsItem* parent);

    int width() const;

private:
    GraphicsNodePrivate* d_ptr;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
};

class NodeTitle;

class GraphicsNodePrivate final
{
public:
    explicit GraphicsNodePrivate(GraphicsNode* q) : q_ptr(q) {}

    QNodeEditorSocketModel* m_pModel;
    QPersistentModelIndex m_Index;

    // TODO: change pairs of sizes to QPointF, QSizeF, or quadrupels to QRectF

    constexpr static const qreal _hard_min_width  = 150.0;
    constexpr static const qreal _hard_min_height = 120.0;

    QSizeF m_MinSize {150.0, 120.0};

    constexpr static const qreal _top_margin    = 30.0;
    constexpr static const qreal _bottom_margin = 15.0;
    constexpr static const qreal _item_padding  = 5.0;
    constexpr static const qreal _lr_padding    = 10.0;

    constexpr static const qreal _pen_width = 1.0;
    constexpr static const qreal _socket_size = 6.0;

    NodeGraphicsItem* m_pGraphicsItem;

    bool _changed {false};

    QSizeF m_Size {150, 120};

    QPen _pen_default  {QColor("#7F000000")};
    QPen _pen_selected {QColor("#FFFF36A7")};
    QPen _pen_sources  {QColor("#FF000000")};
    QPen _pen_sinks    {QColor("#FF000000")};

    QBrush _brush_title      {QColor("#E3212121")};
    QBrush _brush_background {QColor("#E31a1a1a")};
    QBrush m_brushSources    {QColor("#FFFF7700")};
    QBrush m_brushSinks      {QColor("#FF0077FF")};

    //TODO lazy load, add option to disable, its nice, but sllooowwww
#if 0
    QGraphicsDropShadowEffect *_effect        {new QGraphicsDropShadowEffect()};
#endif
    NodeTitle            *_title_item    {nullptr};
    QGraphicsPixmapItem  *_deco_item     {nullptr};
    CloseButton          *_close_item    {nullptr};
    QGraphicsProxyWidget *_central_proxy {nullptr};

    // Helpers
    void updateGeometry();
    void updateSizeHints();

    GraphicsNode* q_ptr;
};

// Necessary for some compilers...
constexpr const qreal GraphicsNodePrivate::_hard_min_width;
constexpr const qreal GraphicsNodePrivate::_hard_min_height;
constexpr const qreal GraphicsNodePrivate::_top_margin;
constexpr const qreal GraphicsNodePrivate::_bottom_margin;
constexpr const qreal GraphicsNodePrivate::_item_padding;
constexpr const qreal GraphicsNodePrivate::_lr_padding;
constexpr const qreal GraphicsNodePrivate::_pen_width;
constexpr const qreal GraphicsNodePrivate::_socket_size;

class NodeTitle : public QGraphicsTextItem
{
public:
    explicit NodeTitle(GraphicsNodePrivate* parent) : QGraphicsTextItem(parent->m_pGraphicsItem),
    d_ptr(parent) {}

private:
    GraphicsNodePrivate* d_ptr;

protected:
    virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event) override;
    virtual void focusOutEvent(QFocusEvent* event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
};


void NodeTitle::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event);
    setTextInteractionFlags(
        Qt::TextEditorInteraction
    );

    auto cur = textCursor();
    cur.select(QTextCursor::Document);
    setTextCursor(cur);
    setFocus();
}

void NodeTitle::focusOutEvent(QFocusEvent* event)
{
    auto cur = textCursor();
    cur.clearSelection();
    setTextCursor(cur);

    setTextInteractionFlags(
        Qt::NoTextInteraction
    );

    if (event)
        event->accept();
}

void NodeTitle::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        focusOutEvent(nullptr);
        d_ptr->q_ptr->setTitle(toPlainText());
        event->accept();
    }
    else if (event->key() == Qt::Key_Escape) {
        focusOutEvent(nullptr);
        setPlainText(d_ptr->m_Index.data().toString());
        event->accept();
    }

    QGraphicsTextItem::keyPressEvent(event);
}

GraphicsNode::GraphicsNode(QNodeEditorSocketModel* model, const QPersistentModelIndex& index, QGraphicsItem *parent)
: QObject(nullptr), d_ptr(new GraphicsNodePrivate(this))
{
    d_ptr->m_pModel = model;
    d_ptr->m_Index = index;

    d_ptr->m_pGraphicsItem = new NodeGraphicsItem(parent);
    d_ptr->m_pGraphicsItem->d_ptr = d_ptr;
    d_ptr->m_pGraphicsItem->q_ptr = this;

    d_ptr->_deco_item = new QGraphicsPixmapItem(d_ptr->m_pGraphicsItem);
    d_ptr->_deco_item->setPos(2, 4);

    d_ptr->_title_item = new NodeTitle(d_ptr);

    for (auto p : {
      &d_ptr->_pen_default, &d_ptr->_pen_selected,
      &d_ptr->_pen_default, &d_ptr->_pen_selected
    })
        p->setWidth(0);

    d_ptr->m_pGraphicsItem->setFlag(QGraphicsItem::ItemIsMovable);
    d_ptr->m_pGraphicsItem->setFlag(QGraphicsItem::ItemIsSelectable);
    d_ptr->m_pGraphicsItem->setFlag(QGraphicsItem::ItemSendsGeometryChanges);

    // The close button
    d_ptr->_close_item = new CloseButton(d_ptr->m_pGraphicsItem);
    d_ptr->_close_item->setPos(d_ptr->m_Size.width() - d_ptr->_close_item->width(), 0);

    d_ptr->_title_item->setDefaultTextColor(Qt::white);

    d_ptr->_title_item->setPos(20, 0);
    d_ptr->_title_item->setTextWidth(
        d_ptr->m_Size.width()
        - 2*d_ptr->_lr_padding
        - d_ptr->_close_item->width()
        - 20 //icon
    );

#if 0
    d_ptr->_effect->setBlurRadius(13.0);
    d_ptr->_effect->setColor(QColor("#99121212"));

    d_ptr->m_pGraphicsItem->setGraphicsEffect(d_ptr->_effect);
#endif
}


void GraphicsNode::
setTitle(const QString &title)
{
    if (auto m = const_cast<QAbstractItemModel*>(d_ptr->m_Index.model()))
        m->setData(d_ptr->m_Index, title, Qt::DisplayRole);

    d_ptr->m_pGraphicsItem->prepareGeometryChange();
    d_ptr->_title_item->setPlainText(d_ptr->m_Index.data().toString());
}

void GraphicsNode::
setDecoration(const QVariant& deco)
{
    if (!deco.isValid())
        return;

    if (deco.canConvert<QIcon>()) {
        const auto px = qvariant_cast<QIcon>(deco).pixmap(16,16);
        d_ptr->_deco_item->setPixmap(px);
    }
    else if (deco.canConvert<QPixmap>()) {
        d_ptr->_deco_item->setPixmap(qvariant_cast<QPixmap>(deco));
    }
}

void GraphicsNode::
setBackground(const QString& brush)
{
    if (auto m = const_cast<QAbstractItemModel*>(d_ptr->m_Index.model()))
        m->setData(d_ptr->m_Index, QColor(brush), Qt::BackgroundRole);
}

void GraphicsNode::
setBackground(const QBrush& brush)
{
    if (auto m = const_cast<QAbstractItemModel*>(d_ptr->m_Index.model()))
        m->setData(d_ptr->m_Index, brush, Qt::BackgroundRole);
}

void GraphicsNode::
setForeground(const QString& pen)
{
    setForeground(QColor(pen));
}

void GraphicsNode::
setForeground(const QPen& pen)
{
    if (auto m = const_cast<QAbstractItemModel*>(d_ptr->m_Index.model()))
        m->setData(d_ptr->m_Index, pen, Qt::ForegroundRole);

    //FIXME this should be removed once dataChanged really reload it
    const auto fgVar = d_ptr->m_Index.data(Qt::ForegroundRole);

    d_ptr->_title_item->setDefaultTextColor(fgVar.canConvert<QColor>() ?
        qvariant_cast<QColor>(fgVar) : Qt::white
    );

    // Update the palette
    if (d_ptr->_central_proxy) {
        auto pal = d_ptr->_central_proxy->palette();
        pal.setColor(QPalette::Foreground, pen.color());
        d_ptr->_central_proxy->setPalette(pal);
    }
}

void GraphicsNode::
setForeground(const QColor& pen)
{
    if (auto m = const_cast<QAbstractItemModel*>(d_ptr->m_Index.model()))
        m->setData(d_ptr->m_Index, pen, Qt::ForegroundRole);

    //FIXME this should be removed once dataChanged really reload it
    const auto fgVar = d_ptr->m_Index.data(Qt::ForegroundRole);

    d_ptr->_title_item->setDefaultTextColor(fgVar.canConvert<QColor>() ?
        qvariant_cast<QColor>(fgVar) : Qt::white
    );

    // Update the palette
    if (d_ptr->_central_proxy) {
        auto pal = d_ptr->_central_proxy->palette();
        pal.setColor(QPalette::Foreground, pen);
        d_ptr->_central_proxy->setPalette(pal);
    }
}

QString GraphicsNode::
title() const
{
    return d_ptr->m_Index.data().toString();
}

QBrush GraphicsNode::
background() const
{
    return qvariant_cast<QBrush>(d_ptr->m_Index.data(Qt::BackgroundRole));
}

QPen GraphicsNode::
foreground() const
{
    return qvariant_cast<QPen>(d_ptr->m_Index.data(Qt::ForegroundRole));
}

QAbstractItemModel *GraphicsNode::
sinkModel() const
{
    return d_ptr->m_pModel->sinkSocketModel(d_ptr->m_Index);
}

QAbstractItemModel *GraphicsNode::
sourceModel() const
{
    return d_ptr->m_pModel->sourceSocketModel(d_ptr->m_Index);
}

int NodeGraphicsItem::
type() const
{
    return GraphicsNodeItemTypes::TypeNode;
}

QSizeF GraphicsNode::
size() const
{
    return d_ptr->m_Size;
}

QRectF GraphicsNode::
rect() const
{
    return QRectF(
        d_ptr->m_pGraphicsItem->pos(),
        d_ptr->m_Size
    );
}

QGraphicsItem *GraphicsNode::
graphicsItem() const
{
    return d_ptr->m_pGraphicsItem;
}

GraphicsNode::
~GraphicsNode()
{
    Q_ASSERT(!d_ptr->m_pGraphicsItem->scene());
    // The widget proxy doesn't own the widget unless specified
    if (d_ptr->_central_proxy) {
        delete d_ptr->_central_proxy;
        d_ptr->_central_proxy = Q_NULLPTR;

        // Even when removed from the scene, the prepareGeometryChange is
        // required to avoid a crash, don't ask me why
        //setSize(0,0);
    }

    delete d_ptr->_title_item;
    delete d_ptr->_deco_item;
#if 0
    delete d_ptr->_effect;
#endif
    delete d_ptr->m_pGraphicsItem;
    delete d_ptr;
}


QRectF NodeGraphicsItem::
boundingRect() const
{
    return QRectF(
        -d_ptr->_pen_width/2.0 - d_ptr->_socket_size,
        -d_ptr->_pen_width/2.0,
        d_ptr->m_Size.width()  + d_ptr->_pen_width/2.0 + 2.0 * d_ptr->_socket_size,
        d_ptr->m_Size.height() + d_ptr->_pen_width/2.0
    ).normalized();
}


void NodeGraphicsItem::
paint(QPainter *painter, const QStyleOptionGraphicsItem *, QWidget *)
{
    const qreal edge_size = 10.0;
    const qreal title_height = 20.0;

    // path for the caption of this node
    QPainterPath path_title;
    path_title.setFillRule(Qt::WindingFill);
    path_title.addRoundedRect(QRect(0, 0, d_ptr->m_Size.width(), title_height), edge_size, edge_size);
    path_title.addRect(0, title_height - edge_size, edge_size, edge_size);
    path_title.addRect(d_ptr->m_Size.width() - edge_size, title_height - edge_size, edge_size, edge_size);
    painter->setPen(Qt::NoPen);
    painter->setBrush(d_ptr->_brush_title);
    painter->drawPath(path_title.simplified());

    // path for the content of this node
    QPainterPath path_content;
    path_content.setFillRule(Qt::WindingFill);
    path_content.addRoundedRect(QRect(0, title_height, d_ptr->m_Size.width(), d_ptr->m_Size.height() - title_height), edge_size, edge_size);
    path_content.addRect(0, title_height, edge_size, edge_size);
    path_content.addRect(d_ptr->m_Size.width() - edge_size, title_height, edge_size, edge_size);
    painter->setPen(Qt::NoPen);

    const auto bgVar = d_ptr->m_Index.data(Qt::BackgroundRole);

    painter->setBrush(bgVar.canConvert<QBrush>() ?
        qvariant_cast<QBrush>(bgVar) : d_ptr->_brush_background
    );

    painter->drawPath(path_content.simplified());

    // path for the outline
    QPainterPath path_outline = QPainterPath();
    path_outline.addRoundedRect(QRect(0, 0, d_ptr->m_Size.width(), d_ptr->m_Size.height()), edge_size, edge_size);
    painter->setPen(isSelected() ? d_ptr->_pen_selected : d_ptr->_pen_default);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(path_outline.simplified());

    // debug bounding box
#if 0
    QPen debugPen = QPen(QColor(Qt::red));
    debugPen.setWidth(0);
    auto r = boundingRect();
    painter->setPen(debugPen);
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(r);

    painter->drawPoint(0,0);
#endif
}


void NodeGraphicsItem::
mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    // TODO: ordering after selection/deselection cycle
    QGraphicsItem::mousePressEvent(event);

    setZValue(isSelected() ? 1 : 0);
}


void GraphicsNode::
setSize(const qreal width, const qreal height)
{
    setSize(QPointF(width, height));
}


void GraphicsNode::
setSize(const QPointF size)
{
    setSize(QSizeF(size.x(), size.y()));
}


void GraphicsNode::
setSize(const QSizeF size)
{
    d_ptr->m_Size = {
        std::max(d_ptr->m_MinSize.width() , size.width ()),
        std::max(d_ptr->m_MinSize.height(), size.height())
    };

    d_ptr->_changed = true;
    d_ptr->m_pGraphicsItem->prepareGeometryChange();
    d_ptr->updateGeometry();
}

void GraphicsNode::
setRect(const qreal x, const qreal y, const qreal width, const qreal height)
{
    graphicsItem()->setPos(x, y);
    setSize(width, height);
}

void GraphicsNode::
setRect(const QRectF size)
{
    graphicsItem()->setPos(size.topLeft());
    setSize(size.size());
}

QVariant NodeGraphicsItem::
itemChange(GraphicsItemChange change, const QVariant &value)
{
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wswitch"
    switch (change) {
    case QGraphicsItem::ItemSelectedChange:
        setZValue(value.toBool() ? 1 : 0);
        break;
    case QGraphicsItem::ItemPositionChange:
    case QGraphicsItem::ItemPositionHasChanged: {
        auto m = const_cast<QAbstractItemModel*>(d_ptr->m_Index.model());

        m->setData(d_ptr->m_Index, q_ptr->rect(), Qt::SizeHintRole);
    }
        break;

    default:
        break;
    }
    #pragma GCC diagnostic pop
    return QGraphicsItem::itemChange(change, value);
}

void GraphicsNode::update()
{
    d_ptr->_changed = true;
    d_ptr->m_pGraphicsItem->prepareGeometryChange();
    d_ptr->updateGeometry();
}

QModelIndex GraphicsNode::index() const
{
    return d_ptr->m_Index;
}

void GraphicsNode::setIndex(const QModelIndex& idx)
{
    d_ptr->m_Index = idx;
}

QAbstractItemModel* GraphicsNode::model() const
{
    return d_ptr->m_pModel;
}

const QModelIndex GraphicsNode::socketIndex(const QString& name) const
{
    const auto c = model()->rowCount(index());
    for (int i = 0; i < c; i++) {
        auto idx = model()->index(i, 0, index());
        if (idx.data() == name)
            return idx;
    }

    return {};
}

void GraphicsNodePrivate::
updateGeometry()
{
    if (!_changed) return;

    // compute if we have reached the minimum size
    updateSizeHints();

    // close button
    _close_item->setPos(m_Size.width() - _close_item->width(), 0);

    // title
    _title_item->setTextWidth(m_Size.width() - _close_item->width());

    qreal yposSink = _top_margin;
    qreal yposSrc  = m_Size.height() - _bottom_margin;

    const int count = m_pModel->rowCount(m_Index);

    for (int i = 0; i < count; i++) {
        const auto idx = m_pModel->index(i, 0, m_Index);

        // sinks
        if (const auto s = m_pModel->getSinkSocket(idx)) {
            const auto size = s->size();

            s->graphicsItem()->setPos(0, yposSink + size.height()/2.0);
            yposSink += size.height() + _item_padding;

            s->graphicsItem()->setOpacity(s->index().flags() & Qt::ItemIsEnabled ?
                1.0 : 0.1
            );
        }

        // sources
        if (const auto s = m_pModel->getSourceSocket(idx)) {
            const auto size = s->size();

            yposSrc -= size.height();
            s->graphicsItem()->setPos(m_Size.width(), yposSrc + size.height()/2.0);
            yposSrc -= _item_padding;

            s->graphicsItem()->setOpacity(s->index().flags() & Qt::ItemIsEnabled ?
                1.0 : 0.1
            );
        }
    }

    // central widget
    if (_central_proxy) {
        _central_proxy->setGeometry({
            _lr_padding,
            yposSink,
            m_Size.width() - 2.0 * _lr_padding,
            yposSrc - yposSink
        });
    }

    _changed = false;
}

void GraphicsNode::
setCentralWidget (QWidget *widget)
{
    if (d_ptr->_central_proxy)
        delete d_ptr->_central_proxy;

    if (!widget) {
        d_ptr->m_pGraphicsItem->prepareGeometryChange();
        d_ptr->updateGeometry();
    }

    d_ptr->_central_proxy = new QGraphicsProxyWidget(d_ptr->m_pGraphicsItem);

    // Update the palette
    auto pal = d_ptr->_central_proxy->palette();

    const auto fgVar = d_ptr->m_Index.data(Qt::ForegroundRole);

    if (fgVar.isValid()) {
        if(fgVar.canConvert<QColor>())
            pal.setColor(QPalette::Foreground, qvariant_cast<QColor>(fgVar));
    }

    pal.setColor(QPalette::Background, Qt::transparent);
    d_ptr->_central_proxy->setPalette(pal);

    d_ptr->_central_proxy->setWidget(widget);
    d_ptr->_changed = true;
    d_ptr->m_pGraphicsItem->prepareGeometryChange();
    d_ptr->updateGeometry();
}


void GraphicsNodePrivate::
updateSizeHints() {
    qreal min_width(0.0), min_height(_top_margin + _bottom_margin);

    const int count = m_pModel->rowCount(m_Index);

    // sinks
    for (int i = 0; i < count; i++) {
        if (const auto s = m_pModel->getSinkSocket(m_pModel->index(i, 0, m_Index))) {
            auto size = s->minimalSize();

            min_height += size.height() + _item_padding;
            min_width   = std::max(size.width(), min_width);
        }
    }

    // central widget
    if (_central_proxy) {
        if (const auto wgt = _central_proxy->widget()) {
            // only take the size hint if the value is valid, and if
            // the minimumSize is not set (similar to
            // QWidget/QLayout standard behavior
            const auto sh = wgt->minimumSizeHint();
            const auto sz = wgt->minimumSize();

            if (sh.isValid()) {
                min_height += (sz.height() > 0) ? sz.height() : sh.height();

                min_width = std::max(
                    qreal((sz.width() > 0) ? sz.width() : sh.width())
                        + 2.0*_lr_padding,
                    min_width
                );

            } else {
                min_height += sh.height();
                min_width   = std::max(
                    qreal(sh.width()) + 2.0*_lr_padding,
                    min_width
                );
            }
        }
    }

    // sources
    for (int i = 0; i < count; i++) {
        if (const auto s = m_pModel->getSourceSocket(m_pModel->index(i, 0, m_Index))) {
            const auto size = s->minimalSize();

            min_height += size.height() + _item_padding;
            min_width   = std::max(size.width(), min_width);
        }
    }

    m_MinSize = {
        std::max(min_width, _hard_min_width  ),
        std::max(min_height, _hard_min_height)
    };

    // prevent the scene from being out of sync
    if (m_Size.width() < min_width || m_Size.height() < min_height) {
        m_pGraphicsItem->prepareGeometryChange();

        m_Size = {
            std::max(min_width , m_Size.width ()),
            std::max(min_height, m_Size.height())
        };

        // Will cause the socket and edges to be updated
        //FIXME Q_EMIT m_pModel->dataChanged(m_Index, m_Index);
    }
}

CloseButton::CloseButton(NodeGraphicsItem* parent) : QGraphicsTextItem(parent),
    d_ptr(parent->d_ptr)
{
    setDefaultTextColor(Qt::white);
    setHtml(QStringLiteral("<b>❌</b>"));
    setTextWidth(width());
}

int CloseButton::width() const
{
    static int closeWidth = 0;
    if (!closeWidth)
        closeWidth = QFontMetrics(font()).height();

    return closeWidth;
}

void CloseButton::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    d_ptr->m_pModel->removeRow(d_ptr->m_Index.row(), d_ptr->m_Index.parent());
    event->accept();
}
