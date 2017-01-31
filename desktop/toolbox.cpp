#include "toolbox.h"

#include "delegates/categorizeddelegate.h"
#include "delegates/autocompletiondelegate.h"


ToolBox::ToolBox(QWidget* parent) : QDockWidget(parent)
{
    setupUi(this);

    setWindowTitle(tr("Tool box"));
    auto del = new CategorizedDelegate(m_pToolBox);
    del->setChildDelegate(new AutoCompletionDelegate());
    m_pToolBox->setItemDelegate(del);
    m_pToolBox->setIndentation(5);
}

ToolBox::~ToolBox()
{

}

void ToolBox::setModel(QAbstractItemModel* m)
{
    m_pToolBox->setModel(m);
    m_pToolBox->expandAll();
}

void ToolBox::expandAll()
{
    m_pToolBox->expandAll();
}

void ToolBox::slotDoubleClicked(const QModelIndex& idx)
{
    Q_EMIT doubleClicked(idx);
}
