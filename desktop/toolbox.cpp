#include "toolbox.h"

#include <QtCore/QDebug>
#include <krecursivefilterproxymodel.h>

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

    m_pProxy = new KRecursiveFilterProxyModel(this);
    m_pProxy->setFilterCaseSensitivity(Qt::CaseInsensitive);
    m_pToolBox->setModel(m_pProxy);

    m_pAction = new QAction(this);
    m_pAction->setText(tr("Find tools"));
    m_pAction->setIcon(QIcon::fromTheme(QStringLiteral("search")));
    connect(m_pAction, &QAction::triggered, this, &ToolBox::actionTrigger);
}

ToolBox::~ToolBox()
{

}

void ToolBox::setModel(QAbstractItemModel* m)
{
    m_pProxy->setSourceModel(m);
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

void ToolBox::setFilterText(const QString& text)
{
    m_pProxy->setFilterFixedString(text);
    expandAll();
}

QAction* ToolBox::searchAction() const
{
    return m_pAction;
}

void ToolBox::actionTrigger()
{
    lineEdit->selectAll();
    lineEdit->setFocus(Qt::ShortcutFocusReason);
}
