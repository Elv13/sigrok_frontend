#pragma once

#include <QtWidgets/QDockWidget>

#include "ui_docktitle.h"

class QAbstractItemModel;
class QItemSelectionModel;

class WidgetGroupModel;

class DockTitle : public QDockWidget, public Ui_DockTitle
{
    Q_OBJECT
public:
    explicit DockTitle(QWidget* parent = nullptr);
    virtual ~DockTitle();

    void setText(const QString& t);
    void setModel(WidgetGroupModel* m);

private:
    WidgetGroupModel* m_pModel {nullptr};

private Q_SLOTS:
    void slotModelChanged();
    void slotSelectionChanged();
};
