#include "mainwindow.h"
#include "ui_appbar.h"
#include "ui_titlebar.h"

#include <QtWidgets/QPushButton>
#include <QtWidgets/QDockWidget>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QTableView>
#include <QtCore/QFile>
#include <QRemoteObjectNode>
#include <QAbstractItemModelReplica>
#include "widgets/lcdmeter.h"
#include "widgets/controls.h"
#include "widgets/curvechart.h"
#include "widgets/meter.h"
#include "common/pagemanager.h"

#include "meterdata_replica.h"

#include "qrc_mobile.cpp"

QWidget* MainWindow::createTitlebar(QWidget* parent)
{
    auto ret = new QWidget(parent);

    auto ui2 = new Ui_Titlebar();
    ui2->setupUi(ret);

    return ret;
}

void MainWindow::initStyle()
{

    // Use a dark material color theme
    QPalette p = QApplication::palette();

    p.setColor(QPalette::Window       , QColor("#303030"));
    p.setColor(QPalette::WindowText   , QColor("#FFFFFF"));
    p.setColor(QPalette::Foreground   , QColor("#FFFFFF"));
    p.setColor(QPalette::Base         , QColor("#424242"));
//     p.setColor(QPalette::AlternateBase, QColor(""));
//     p.setColor(QPalette::ToolTipBase  , QColor(""));
//     p.setColor(QPalette::ToolTipText  , QColor(""));
    p.setColor(QPalette::Text         , QColor("#FFFFFF"));
    p.setColor(QPalette::Button       , QColor("#CCCCCC"));
    p.setColor(QPalette::ButtonText   , QColor("#FFFFFF"));
//     p.setColor(QPalette::BrightText   , QColor(""));
    QApplication::setPalette(p);

    enableEditMode(false);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{

    initStyle();

    QToolBar* tb = new QToolBar(this);
    tb->setStyleSheet("background:#212121;height:2em;");
    addToolBar(Qt::TopToolBarArea, tb);

    auto apw = new QWidget(tb);
    auto ui2 = new Ui_AppBar();
    ui2->setupUi(apw);
    tb->addWidget(apw);
    connect(ui2->m_pEdit, &QToolButton::toggled, this, &MainWindow::enableEditMode);

    registry = new QRemoteObjectNode(QUrl(QStringLiteral("tcp://10.10.10.136:2223")));

    registry->waitForRegistry(); //TODO remove

    const bool ret1 = registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2224")));
    const bool ret2 = registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2225")));

    m_pPageModelReplica = registry->acquireModel(QStringLiteral("PageManager"));

    connect(m_pPageModelReplica, &QAbstractItemModel::dataChanged, [this](const QModelIndex& tl,const QModelIndex& br) {

        for (int i = tl.row(); i <= br.row(); i++) {
            const QPersistentModelIndex idx = m_pPageModelReplica->index(i, 0);
            createSection(idx);
        }
    });

    connect(m_pPageModelReplica, &QAbstractItemModel::modelReset, [this]() {
        slotPageInserted({}, 0, m_pPageModelReplica->rowCount()-1);
    });

    connect(m_pPageModelReplica, &QAbstractItemModel::rowsAboutToBeRemoved, [this](const QModelIndex& tl, int first, int last) {
        for (int i = first; i <= last; i++) {
            const auto uid = m_pPageModelReplica->index(i, 0).data(PageManager::Role::REMOTE_OBJECT_UID).toString();
            //FIXME Q_ASSERT(!uid.isEmpty());

            if (m_lDocks.contains(uid)) {
                removeDockWidget(m_lDocks[uid]);
                m_lDocks[uid] = nullptr;
            }
        }
    });

}

void MainWindow::addWidget(RemoteWidget wdg)
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

    if (t.contains(wdg.m_Type)) {
        auto w = t[wdg.m_Type](wdg.m_pModel);
        auto dock = new QDockWidget(wdg.m_Name, this);
//         dock->setTitleBarWidget(createTitlebar(dock));

        dock->setWidget(w);

        addDockWidget(Qt::LeftDockWidgetArea, dock);
        m_lDocks[wdg.m_Uid] = dock;
    }
}

void MainWindow::createSection(const QPersistentModelIndex& idx)
{
    if (m_hLoaded[idx])
        return;

    const QString name = idx.data(PageManager::Role::REMOTE_MODEL_NAME ).toString();
    const QString type = idx.data(PageManager::Role::REMOTE_WIDGET_TYPE).toString();
    const QString uid  = idx.data(PageManager::Role::REMOTE_OBJECT_UID ).toString();

    if (name.isEmpty() || type.isEmpty() || uid.isEmpty())
        return;

    auto m = registry->acquireModel(name);

    if (!m)
        return;

    addWidget({
        type,
        name,
        uid,
        m
    });

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

void MainWindow::enableEditMode(bool edit)
{
    static const QResource ss(":/css/default.css");
    static const QResource ess(":/css/edit.css");

    static QByteArray normalCss((char*)ss.data(),ss.size());
    static QByteArray editCss = normalCss+QByteArray((char*)ess.data(),ess.size());

    setStyleSheet(edit ? editCss : normalCss);

    Q_FOREACH(QDockWidget* dock, m_lDocks) {
        if (dock) {
            dock->setFeatures( edit ?
                QDockWidget::DockWidgetMovable : QDockWidget::NoDockWidgetFeatures
            );
            if (dock->titleBarWidget()) {
//                 dock->titleBarWidget()->setMaximumSize(9999999, edit ? 99999999 : 0);
                //dock->titleBarWidget()->setVisible(edit);
            }
        }
    }
}

MainWindow::~MainWindow()
{
    //delete ui;
}

