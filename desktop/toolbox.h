#pragma once

#include <QtWidgets/QDockWidget>

class KRecursiveFilterProxyModel;

#include "ui_toolbox.h"

class QAbstractItemModel;
class QModelIndex;

class ToolBox : public QDockWidget, public Ui_ToolBox
{
    Q_OBJECT
public:
    explicit ToolBox(QWidget* parent);
    virtual ~ToolBox();

    void setModel(QAbstractItemModel* m);

private Q_SLOTS:
    void slotDoubleClicked(const QModelIndex& idx);

public Q_SLOTS:
    void expandAll();
    void setFilterText(const QString& text);

Q_SIGNALS:
    void doubleClicked(const QModelIndex& idx);

private:
    KRecursiveFilterProxyModel* m_pProxy;
};
