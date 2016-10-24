#pragma once

#include <QtWidgets/QStyledItemDelegate>

class QTreeView;

///CategorizedDelegate: A categorized tree delegate
class CategorizedDelegate : public QStyledItemDelegate
{
   Q_OBJECT
public:
   //Constructor
   explicit CategorizedDelegate(QTreeView* widget);
   virtual ~CategorizedDelegate();

   //Reimplemented methods
   virtual QSize sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const override;
   virtual void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override;

   //Setters
   void setChildDelegate(QStyledItemDelegate* childDelegate);
   void setChildChildDelegate(QStyledItemDelegate* childDelegate);

private:
   //Helpers
   void drawSimpleCategory(const QModelIndex &index, int sortRole, const QStyleOption &option, QPainter *painter) const;

   //Attributes
   const int  m_LeftMargin ;
   QStyledItemDelegate* m_pChildDelegate     ;
   QStyledItemDelegate* m_pChildChildDelegate;
};
