#pragma once

#include <QtWidgets/QStyledItemDelegate>

class ColorDelegate : public QStyledItemDelegate
{
   Q_OBJECT
public:
   //Constructor
   explicit ColorDelegate(QObject* parent = nullptr);
   virtual ~ColorDelegate();
   virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

};
