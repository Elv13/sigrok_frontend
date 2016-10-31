#include "range.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "ui_rangeselection.h"
#include "delegates/categorizeddelegate.h"

#include "../proxies/rangeproxy.h"

Range::Range(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
    auto del = new CategorizedDelegate(treeView);
    treeView->setItemDelegate(del);
    treeView->setIndentation(0);
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
    treeView->setModel(p);
//     p->setWidget(treeView);

    setColumnWidgetFactory(0, true, [this](int row) -> QWidget* {

        auto w = new QWidget();

        auto ui = new Ui_RangeSelection();
        ui->setupUi(w);
        ui->comboBox->setModel(m_pProxy->delimiterModel());
        connect(ui->comboBox, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
            , [this, ui, row]() {
                ui->doubleSpinBox->setEnabled(ui->comboBox->currentIndex());
                m_pProxy->setData(
                    m_pProxy->index(0,0, m_pProxy->index(row,0)),
                    ui->comboBox->currentIndex(),
                    (int)RangeProxy::Role::RANGE_DELIMITER
                );
            }
        );
        connect(ui->doubleSpinBox, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this, ui, row](double v) {
                m_pProxy->setData(
                    m_pProxy->index(0,0, m_pProxy->index(row,0)),
                    v,
                    (int)RangeProxy::Role::RANGE_VALUE
                );
            }
        );

        return w;
    });

    slotAjustColumns();
    treeView->expandAll();
    connect(p, &RangeProxy::layoutChanged, this, &Range::slotAjustColumns);
    connect(p, &RangeProxy::columnsInserted, this, &Range::slotAjustColumns);
    connect(p, &RangeProxy::rowsInserted, this, &Range::slotAjustColumns);
}

void Range::slotAjustColumns()
{
    if (treeView->header()) {
        treeView->header()->setHidden(true);
        treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        treeView->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
        treeView->header()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    }

    applyWidget({}, m_lWidgetFactories);

    // Let the layout some time to adjust
    QTimer::singleShot(0, [this](){treeView->expandAll();});
}

void Range::setColumnWidgetFactory(int col, bool isRange, std::function<QWidget*(int)> w)
{
    const int cc = m_pProxy->columnCount();
    if (col > cc) return;

    //FIXME hack
    auto list = &(isRange ? m_lWidgetFactoriesChild : m_lWidgetFactories);

    list->resize(cc);

    (*list)[col] = w;
    applyWidget({}, m_lWidgetFactories);
}

void Range::applyWidget(const QModelIndex& root, QVector< std::function<QWidget*(int)> >& f)
{
    qDebug() << "RELOAD";
    // Create all missing widgets
    for (int i=0; i < m_pProxy->rowCount(root); i++) {
        for (int j = 0; j < m_pProxy->columnCount(root); j++) {
            const QModelIndex idx = m_pProxy->index(i,j, root);
            auto w = treeView->indexWidget(idx);

            if ((!w) && f.size() > j && f[j]) {
                w = f[j](root.row());
                treeView->setIndexWidget(idx, w);
            }

            if (idx.isValid())
                applyWidget(idx, m_lWidgetFactoriesChild);
        }
    }
}
