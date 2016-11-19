#ifndef CATEGORIZED_TREE_H
#define CATEGORIZED_TREE_H

#include <QtWidgets/QTreeView>

class CategorizedTree : public QTreeView
{
   Q_OBJECT
public:
   explicit CategorizedTree(QWidget* parent = nullptr);

protected:
   virtual void drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const override;
};

#endif
