#include "docktitle.h"

#include <QtCore/QItemSelectionModel>
#include <QtCore/QDebug>
#include <QtWidgets/QMainWindow>

#include "common/widgetgroupmodel.h"

DockTitle::DockTitle(QWidget* parent) :  QDockWidget(parent)
{
    auto titlebar = new QWidget(this);
    setupUi(titlebar);
    setTitleBarWidget(titlebar);

    connect(m_pGroup, SIGNAL(currentIndexChanged(int)), this, SLOT(slotSelectionChanged()));
}

DockTitle::~DockTitle()
{

}

void DockTitle::setText(const QString& t)
{
    m_pTitle->setText(t);
}

void DockTitle::setModel(WidgetGroupModel* m)
{
    m_pGroup->setModel(m);

    m_pModel = m;

    connect(m, &QAbstractItemModel::rowsInserted, this, &DockTitle::slotModelChanged);
    connect(m, &QAbstractItemModel::rowsRemoved, this, &DockTitle::slotModelChanged);
    slotModelChanged();

}

void DockTitle::slotModelChanged()
{
    m_pGroup->setVisible(m_pGroup->model()->rowCount() > 1);
}

void DockTitle::slotSelectionChanged()
{
    if ((!m_pGroup->count()) || !m_pModel)
        return;

    const auto idx = m_pGroup->model()->index(m_pGroup->currentIndex(), 0);
    auto mw = m_pModel->mainWindow(idx);

    Q_ASSERT(mw);

    setParent(mw);
    mw->addDockWidget(Qt::LeftDockWidgetArea, this);
}
