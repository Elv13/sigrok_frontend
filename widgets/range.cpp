#include "range.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "ui_rangeselection.h"
#include "delegates/categorizeddelegate.h"

#include "../proxies/rangeproxy.h"
#include "../proxies/filtertoplevelproxy.h"

Range::Range(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
    auto del = new CategorizedDelegate(treeView);
    treeView->setItemDelegate(del);
    treeView->setIndentation(0);
    m_pFiltered = new FilterTopLevelProxy(nullptr);
    treeView->setModel(m_pFiltered);
}

Range::~Range()
{

}

RangeProxy* Range::rangeProxy() const
{
    return m_pProxy;
}

void Range::setRangeProxy(RangeProxy* p)
{
    m_pProxy = p;
    m_pFiltered->setSourceModel(p);
    comboBox->setModel(p);
    comboBox->setCurrentIndex(p->rowCount()-1);

    setColumnWidgetFactory(0, [this](const QPersistentModelIndex& idx) -> QWidget* {

        auto w = new QWidget();

        auto ui = new Ui_RangeSelection();
        ui->setupUi(w);
        ui->comboBox->setModel(m_pProxy->delimiterModel());
        ui->comboBox->setCurrentIndex(
            (int) qvariant_cast<RangeProxy::Delimiter>(
                idx.data((int)RangeProxy::Role::RANGE_DELIMITER)
            )
        );
        connect(ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
            , [this, ui, idx]() {
                ui->doubleSpinBox->setEnabled(ui->comboBox->currentIndex());

                m_pProxy->setData(
                    idx,
                    ui->comboBox->currentIndex(),
                    (int)RangeProxy::Role::RANGE_DELIMITER
                );
            }
        );
        connect(ui->doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this, ui, idx](double v) {
                m_pProxy->setData(
                    idx,
                    v,
                    (int)RangeProxy::Role::RANGE_VALUE
                );
            }
        );

        return w;
    });

    slotAjustColumns();
    treeView->expandAll();
    connect(p, &RangeProxy::layoutChanged  , this, &Range::slotAjustColumns);
    connect(p, &RangeProxy::columnsInserted, this, &Range::slotAjustColumns);
    connect(p, &RangeProxy::rowsInserted   , this, &Range::slotAjustColumns);
}

void Range::slotAjustColumns()
{
    if (!treeView->header())
        return;

    treeView->header()->setHidden(true);
    treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch         );
    treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    applyWidget({}, m_lWidgetFactories);

    // Let the layout some time to adjust
    QTimer::singleShot(0, [this](){treeView->expandAll();});
}

void Range::setColumnWidgetFactory(int col, std::function<QWidget*(const QPersistentModelIndex& idx)> w)
{
    const int cc = m_pProxy->columnCount();
    if (col > cc) return;

    //FIXME hack
    auto list = &(true ? m_lWidgetFactoriesChild : m_lWidgetFactories);

    list->resize(cc);

    (*list)[col] = w;
    applyWidget({}, m_lWidgetFactories);
}

void Range::slotAddClicked()
{
    if (!m_pProxy) return;
    m_pProxy->addFilter(m_pProxy->index(comboBox->currentIndex(),0));
}

void Range::slotAllColumns(bool val)
{
    if (!m_pProxy) return;
    m_pProxy->setMatchAllFilters(val);
}

void Range::applyWidget(const QModelIndex& root, QVector< std::function<QWidget*(const QPersistentModelIndex& idx)> >& f)
{
    qDebug() << "RELOAD";
    // Create all missing widgets
    for (int i=0; i < m_pFiltered->rowCount(root); i++) {
        for (int j = 0; j < m_pFiltered->columnCount(root); j++) {
            const QModelIndex idx = m_pFiltered->index(i,j, root);
            auto w = treeView->indexWidget(idx);

            if ((!w) && f.size() > j && f[j]) {
                w = f[j](m_pFiltered->mapToSource(idx));
                treeView->setIndexWidget(idx, w);
            }

            if (idx.isValid())
                applyWidget(idx, m_lWidgetFactoriesChild);
        }
    }
}
