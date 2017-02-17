#include "range.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>
#include <QtCore/QItemSelectionModel>
#include <QtWidgets/QDialogButtonBox>

#include "Qt-Color-Widgets/include/ColorWheel"

#include "delegates/categorizeddelegate.h"

#include "proxies/rangeproxy.h"
#include "proxies/filtertoplevelproxy.h"
#include "ui_rangeselection.h"
#include "ui_range.h"
#include "delegates/colordelegate.h"

Range::Range(QWidget* parent) : QWidget(parent)
{
    Ui_Range ui;
    ui.setupUi(this);
    m_pTree = ui.treeView;
    m_pColumn = ui.comboBox;
    auto del = new CategorizedDelegate(m_pTree);
    del->setChildDelegate(new ColorDelegate(this));
    m_pTree->setItemDelegate(del);
    m_pTree->setIndentation(0);
    m_pFiltered = new FilterTopLevelProxy(nullptr);
    m_pTree->setModel(m_pFiltered);

    m_pLayout = ui.verticalLayout_2;
    m_pMainWidget = ui.m_pMainWidget;

    connect(m_pFiltered, &FilterTopLevelProxy::layoutChanged  , this, &Range::slotAjustColumns);
    connect(m_pFiltered, &FilterTopLevelProxy::columnsInserted, this, &Range::slotAjustColumns);
    connect(m_pFiltered, &FilterTopLevelProxy::rowsInserted   , this, &Range::slotAjustColumns);
}

Range::~Range()
{
    delete m_pFiltered;
}

RangeProxy* Range::rangeProxy() const
{
    return m_pProxy;
}

void Range::setRangeProxy(RangeProxy* p)
{
    m_pProxy = p;

    // Act on clicks
    auto sm = m_pTree->selectionModel();
    connect(sm, &QItemSelectionModel::currentChanged, [this, p, sm](const QModelIndex& idx) {
        if (m_Mutex || !idx.isValid() || !idx.parent().isValid())
            return;

        m_Mutex = true;

        auto m = sm->model();

        if (sm->model() != p) {
            while (m && m != p && qobject_cast<QAbstractProxyModel*>(m))
                m = qobject_cast<QAbstractProxyModel*>(m)->sourceModel();
        }

        Q_ASSERT(m == p);

        QPersistentModelIndex pIdx(idx);

        QTimer::singleShot(0,[this, pIdx, sm, p]() {
            sm->clearCurrentIndex();
            sm->clearSelection();
            sm->setCurrentIndex({}, QItemSelectionModel::Clear);

            m_Mutex = false;
            Q_ASSERT(!sm->currentIndex().isValid());

            if (!m_pColorWheel) {
                m_pColorWheel = new color_widgets::ColorWheel(this);
                m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);

                m_pLayout->addWidget(m_pColorWheel);
                m_pLayout->addWidget(m_pButtonBox);

                connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &Range::okClicked);
                connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &Range::cancelClicked);
            }
            else {
                m_pColorWheel->setVisible(true);
                m_pButtonBox->setVisible(true);
            }
            m_pMainWidget->setHidden(true);
            m_CurrentIdx = pIdx;
        });
    });

    m_pFiltered->setSourceModel(p);

    m_pColumn->setModel(p);

    m_pColumn->setCurrentIndex(0);

    m_pColumn->setCurrentIndex(p->rowCount()-1);


    setColumnWidgetFactory(0, [this](const QPersistentModelIndex& idx) -> QWidget* {

        auto w = new QWidget();

        Ui_RangeSelection ui;
        ui.setupUi(w);
        ui.comboBox->setModel(m_pProxy->delimiterModel());
        ui.comboBox->setCurrentIndex(
            (int) qvariant_cast<RangeProxy::Delimiter>(
                idx.data((int)RangeProxy::Role::RANGE_DELIMITER)
            )
        );

        auto cbb = ui.comboBox;
        auto spb = ui.doubleSpinBox;

        connect(cbb, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged)
            , [this, cbb, spb, idx]() {
                spb->setEnabled(cbb->currentIndex());

                m_pProxy->setData(
                    idx,
                    cbb->currentIndex(),
                    (int)RangeProxy::Role::RANGE_DELIMITER
                );
            }
        );
        connect(spb, static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            [this, idx](double v) {
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

    if(m_pColumn->currentIndex() == -1)
        m_pColumn->setCurrentIndex(m_pProxy->rowCount()-1);

    // Let the layout some time to adjust
    QTimer::singleShot(0, [this](){m_pTree->expandAll();});
}

void Range::setColumnWidgetFactory(int col, const std::function<QWidget*(const QPersistentModelIndex& idx)>& w)
{
    const int cc = m_pProxy->columnCount();
    if (col > cc)
        return;

    //FIXME hack
    auto list = &(true ? m_lWidgetFactoriesChild : m_lWidgetFactories);

    list->resize(cc);

    (*list)[col] = w;
    applyWidget({}, m_lWidgetFactories);
}

void Range::slotAddClicked()
{
    if (!m_pProxy) return;

    Q_ASSERT(m_pColumn->count() == m_pProxy->rowCount());

//     for (int i = 0; i < m_pColumn->count(); i++)
//         qDebug() << "I" << i << m_pColumn->itemText(i);

    m_pProxy->addFilter(m_pProxy->index(m_pColumn->currentIndex(),0));
}

void Range::slotAllColumns(bool val)
{
    if (!m_pProxy) return;
    m_pProxy->setMatchAllFilters(val);
}

void Range::applyWidget(const QModelIndex& root, QVector< std::function<QWidget*(const QPersistentModelIndex& idx)> >& f)
{
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

void Range::okClicked()
{
    if (m_CurrentIdx.isValid()) {
        if (m_CurrentIdx.column()==1)
            m_pTree->model()->setData(m_CurrentIdx, m_pColorWheel->color(), Qt::BackgroundRole);
        else
            m_pTree->model()->setData(m_CurrentIdx, m_pColorWheel->color(), Qt::ForegroundRole);
    }

    cancelClicked();
}

void Range::cancelClicked()
{
    m_pColorWheel->setVisible(false);
    m_pButtonBox->setVisible (false);
    m_pMainWidget->setHidden (false);
    m_CurrentIdx = QModelIndex();
}
