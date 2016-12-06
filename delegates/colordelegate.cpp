#include "colordelegate.h"

//Qt
#include <QtGui/QPainter>
#include <QtCore/QDebug>

ColorDelegate::ColorDelegate(QObject* parent)
   : QStyledItemDelegate(parent)
{
}

ColorDelegate::~ColorDelegate()
{
}

///Draw the category (and the first child)
void ColorDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_ASSERT(index.isValid());

    if (index.column() < 1)
        return QStyledItemDelegate::paint(painter, option, index);

    const auto bg = index.data(Qt::BackgroundRole);
    if (bg.canConvert<QColor>()) {
        painter->setBrush(qvariant_cast<QColor>(bg));
    }

    const int size = std::min(option.rect.width(), option.rect.height())-6;

    painter->drawRoundedRect(option.rect.x() + 3, option.rect.y() + 3, size, size,4, 4);


}
