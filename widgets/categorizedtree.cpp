#include "categorizedtree.h"

CategorizedTree::CategorizedTree(QWidget* parent) : QTreeView(parent)
{}

void CategorizedTree::drawBranches(QPainter* painter, const QRect& rect, const QModelIndex& index) const
{
  Q_UNUSED( painter )
  Q_UNUSED( rect    )
  Q_UNUSED( index   )
}
