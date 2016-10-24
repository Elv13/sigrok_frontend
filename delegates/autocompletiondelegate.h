#pragma once

#include <QtWidgets/QStyledItemDelegate>

class AutoCompletionDelegate : public QStyledItemDelegate
{
   Q_OBJECT
public:
   explicit AutoCompletionDelegate(int role = Qt::UserRole);

   virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;
private:
   int m_Role;
};
