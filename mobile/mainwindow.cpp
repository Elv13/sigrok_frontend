#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QDockWidget>
#include <QRemoteObjectNode>
#include <QAbstractItemModelReplica>
#include "widgets/lcdmeter.h"
#include "widgets/controls.h"
#include "widgets/curvechart.h"
#include "widgets/meter.h"
#include "common/pagemanager.h"

#include "meterdata_replica.h"

void MainWindow::addDock(QWidget* w, const QString& title)
{
    static QHash<QString, int> uniqueNames;

    ui->m_pWidgetsL->addWidget(w);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    auto mw = new QWidget(this);
    setCentralWidget(mw);

    ui->setupUi(mw);

    connect(PageManager::instance(), &PageManager::pageAdded, this, &MainWindow::addDock);

    registry = new QRemoteObjectNode(QUrl(QStringLiteral("tcp://10.10.10.136:2223")));

    registry->waitForRegistry(); //TODO remove

    const bool ret1 = registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2224")));
    const bool ret2 = registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2225")));

    m_pPageModelReplica = registry->acquireModel(QStringLiteral("PageManager"));
//     ui->pushButton->setText(model == nullptr ? "NO" : "YES"+ (QString::number(model->rowCount())));
    ui->tableView->setModel(m_pPageModelReplica);

    connect(m_pPageModelReplica, &QAbstractItemModel::modelReset, [this]() {
        ui->pushButton->setText(
            "MODEL RESET "+QString::number(m_pPageModelReplica->rowCount())
        );
        slotPageInserted({}, 0, m_pPageModelReplica->rowCount()-1);
    });

    connect(m_pPageModelReplica, &QAbstractItemModel::dataChanged, [this](const QModelIndex& tl,const QModelIndex& br) {

        for (int i = tl.row(); i <= br.row(); i++) {
            const QPersistentModelIndex idx = m_pPageModelReplica->index(i, 0);
            createSection(idx);
        }
    });

    connect(m_pPageModelReplica, &QAbstractItemModel::rowsAboutToBeRemoved, [this](const QModelIndex& tl, int first, int last) {
        for (int i = first; i <= last; i++) {
            const auto uid = PageManager::instance()->index(i, 0).data(PageManager::Role::REMOTE_OBJECT_UID).toString();

            if (m_lDocks.contains(uid)) {
                ui->m_pWidgetsL->removeWidget(m_lDocks[uid]);
                m_lDocks[uid] = nullptr;
            }
        }
    });

}

void MainWindow::addWidget(const QString& type, QAbstractItemModel* model, const QString& uid)
{
    static QHash<QString, std::function<QWidget*(QAbstractItemModel*)> > t {
        {"lcdmeter_node", [this](QAbstractItemModel* model) {
            auto lcdw = new LCDMeter(this);
            lcdw->setModel(model);
            return lcdw;
        }},
        {"table_node", [this](QAbstractItemModel* m) {
            auto tv = new QTableView(this);
            tv->setModel(m);
            return tv;
        }},
        {"remoteaction_node", [this](QAbstractItemModel* model) {
            auto ctrl = new Controls(this);
            ctrl->setModel(model);
            return ctrl;
        }},
        {"meter_node", [this](QAbstractItemModel* model) {
            auto meter = new Meter(this);
            meter->setModel(model);
            return meter;
        }},
        {"curvedchart_node", [this](QAbstractItemModel* model) {
            auto meter = new CurveChart(this);
            meter->setModel(model);
            return meter;
        }},
    };

    if (t.contains(type)) {
        auto w = t[type](model);
//         ui->m_pWidgetsL->addWidget(w);


        auto dock = new QDockWidget("ASDSAD", this);

        dock->setWidget(w);
//         dock->setObjectName("ASDSAD"+QString::number(uniqueNames["ASDSAD"]++));

        addDockWidget(Qt::BottomDockWidgetArea, dock);
        m_lDocks[uid] = w;
    }
}

void MainWindow::createSection(const QPersistentModelIndex& idx)
{
    if (m_hLoaded[idx])
        return;

    const QString name = idx.data(PageManager::Role::REMOTE_MODEL_NAME ).toString();
    const QString type = idx.data(PageManager::Role::REMOTE_WIDGET_TYPE).toString();
    const QString uid  = idx.data(PageManager::Role::REMOTE_OBJECT_UID ).toString();

    if (name.isEmpty() || type.isEmpty())
        return;
    Q_ASSERT(!type.isEmpty());

    auto m = registry->acquireModel(name);

    if (!m)
        return;

    addWidget(type, m, uid);

    m_hLoaded[idx] = true;
}

void MainWindow::slotPageInserted(const QModelIndex&, int start, int end)
{
    for (int i = start; i <= end; i++) {
        const auto idx = m_pPageModelReplica->index(i, 0);
        Q_ASSERT(idx.isValid());
        const QString remModel = idx.data(PageManager::Role::REMOTE_MODEL_NAME).toString();
        const QString type = idx.data(PageManager::Role::REMOTE_WIDGET_TYPE).toString();

        if (!remModel.isEmpty())
            createSection(idx);
    }
}

MainWindow::~MainWindow()
{
    //delete ui;
}

