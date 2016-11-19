#include "range.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include "delegates/categorizeddelegate.h"

#include "proxies/rangeproxy.h"
#include "proxies/filtertoplevelproxy.h"
#include "ui_rangeselection.h"
#include "ui_range.h"

Range::Range(QWidget* parent) : QWidget(parent)
{
    Ui_Range ui;
    ui.setupUi(this);
    m_pTree = ui.treeView;
    m_pColumn = ui.comboBox;
    auto del = new CategorizedDelegate(m_pTree);
    m_pTree->setItemDelegate(del);
    m_pTree->setIndentation(0);
    m_pFiltered = new FilterTopLevelProxy(nullptr);
    m_pTree->setModel(m_pFiltered);

    connect(m_pFiltered, &FilterTopLevelProxy::layoutChanged  , this, &Range::slotAjustColumns);
    connect(m_pFiltered, &FilterTopLevelProxy::columnsInserted, this, &Range::slotAjustColumns);
    connect(m_pFiltered, &FilterTopLevelProxy::rowsInserted   , this, &Range::slotAjustColumns);
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
    m_pColumn->setModel(p);
    m_pColumn->setCurrentIndex(p->rowCount()-1);

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
    m_pTree->expandAll();
}

void Range::slotAjustColumns()
{
    if (!m_pTree->header())
        return;

    m_pTree->header()->setHidden(true);
    m_pTree->header()->setSectionResizeMode(0, QHeaderView::Stretch         );
    m_pTree->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    m_pTree->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);

    applyWidget({}, m_lWidgetFactories);

    // Let the layout some time to adjust
    QTimer::singleShot(0, [this](){m_pTree->expandAll();});
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
    m_pProxy->addFilter(m_pProxy->index(m_pColumn->currentIndex(),0));
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
            auto w = m_pTree->indexWidget(idx);

            if ((!w) && f.size() > j && f[j]) {
                w = f[j](m_pFiltered->mapToSource(idx));
                m_pTree->setIndexWidget(idx, w);
            }

            if (idx.isValid())
                applyWidget(idx, m_lWidgetFactoriesChild);
        }
    }
}
