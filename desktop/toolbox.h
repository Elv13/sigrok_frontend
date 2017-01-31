#pragma once

#include <QtWidgets/QDockWidget>

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

Q_SIGNALS:
    void doubleClicked(const QModelIndex& idx);
};
