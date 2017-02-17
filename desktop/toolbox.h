#pragma once

#include <QtWidgets/QDockWidget>

class KRecursiveFilterProxyModel;
class QAction;

#include "ui_toolbox.h"

class QAbstractItemModel;
class QModelIndex;
class CategorizedDelegate;

class ToolBox : public QDockWidget, public Ui_ToolBox
{
    Q_OBJECT
public:
    explicit ToolBox(QWidget* parent);
    virtual ~ToolBox();

    QAction* searchAction() const;

    void setModel(QAbstractItemModel* m);

private Q_SLOTS:
    void slotDoubleClicked(const QModelIndex& idx);
    void actionTrigger();

public Q_SLOTS:
    void expandAll();
    void setFilterText(const QString& text);

Q_SIGNALS:
    void doubleClicked(const QModelIndex& idx);

private:
    KRecursiveFilterProxyModel* m_pProxy;
    QAction* m_pAction;
    CategorizedDelegate* m_pDel;
};
