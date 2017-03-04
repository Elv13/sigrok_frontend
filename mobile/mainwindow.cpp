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

#ifdef WITH_QWT
 #include "qwt_thermo.h"
#endif

#include "meterdata_replica.h"

#include "qrc_mobile.cpp"
// #include "qrc_icons.cpp"

#include <QtWidgets/QDialogButtonBox>

#include "Qt-Color-Widgets/include/ColorWheel"

class DockTitleBar : public QLabel
{
    Q_OBJECT

    Q_PROPERTY(QString class READ cssClass)

public:
    explicit DockTitleBar(QWidget* parent, MainWindow* mw) :
        QLabel(QString(), parent), m_pMainWindow(mw){}
    QString cssClass() { return QString("docktitlebar"); }

    MainWindow* m_pMainWindow;
    color_widgets::ColorWheel* m_pColorWheel {nullptr};
    QWidget* m_pDialog;
    bool m_IsBg {false};
    QCheckBox* m_pMain;

    void selectColor(const QColor& base);

public Q_SLOTS:
    void slotMain(bool);
    void slotBg();
    void slotFg();
    void okClicked();
    void cancelClicked();
};

void DockTitleBar::selectColor(const QColor& base)
{
    m_pColorWheel = new color_widgets::ColorWheel(this);
    m_pColorWheel->setColor(base);
    auto m_pButtonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,this);

    m_pDialog = new QWidget(m_pMainWindow);
    m_pDialog->resize(m_pMainWindow->width(), m_pMainWindow->height());
    auto l = new QVBoxLayout(m_pDialog);
    l->addWidget(m_pColorWheel);
    l->addWidget(m_pButtonBox);

    connect(m_pButtonBox, &QDialogButtonBox::accepted, this, &DockTitleBar::okClicked);
    connect(m_pButtonBox, &QDialogButtonBox::rejected, this, &DockTitleBar::cancelClicked);
    m_pDialog->show();
}

void DockTitleBar::okClicked()
{
    auto col = m_pColorWheel->color();

    auto p = parentWidget()->palette();

    if (m_IsBg)
        p.setColor(QPalette::Base, col);
    else
        p.setColor(QPalette::WindowText, col);

    parentWidget()->setPalette(p);

    cancelClicked();
}

void DockTitleBar::cancelClicked()
{
    m_pDialog->setVisible(false);
    delete m_pColorWheel;
    delete m_pDialog;

}

void DockTitleBar::slotMain(bool v)
{
    if (auto dock = qobject_cast<QDockWidget*>(parentWidget())) {
        if (!v) {
            dock->setParent(m_pMainWindow);
            dock->QObject::setParent(m_pMainWindow);
        }

        if (v) {
            // Remove the existing
            if (m_pMainWindow->centralWidget()
                && m_pMainWindow->centralWidget()->layout()
                && m_pMainWindow->centralWidget()->layout()->count()
                && m_pMainWindow->centralWidget()->layout()->itemAt(0)->widget()
            )
                if (auto dock = qobject_cast<QDockWidget*>(
                    m_pMainWindow->centralWidget()->layout()->itemAt(0)->widget())
                ) {
                    qvariant_cast<DockTitleBar*>(
                        dock->property("editTitlebar")
                    )->m_pMain->setChecked(false);
                }

            auto w = new QWidget(m_pMainWindow);
            auto l = new QHBoxLayout(w);
            l->setContentsMargins(0,0,0,0);
            l->addWidget(dock);
            m_pMainWindow->setCentralWidget(w);
        }
        else {
            m_pMainWindow->centralWidget()->layout()->removeWidget(dock);
            m_pMainWindow->setCentralWidget(nullptr);
            m_pMainWindow->addDockWidget(Qt::LeftDockWidgetArea, dock);
        }
    }
}

void DockTitleBar::slotBg()
{
    m_IsBg = true;
    auto p = parentWidget()->palette();

    selectColor(p.color(QPalette::Base));
}

void DockTitleBar::slotFg()
{
    m_IsBg = false;
    auto p = parentWidget()->palette();

    selectColor(p.color(QPalette::WindowText));
}

QWidget* MainWindow::createTitlebar(QWidget* parent, MainWindow* mw)
{
    static const QResource tss(":/css/docktilebar.css");
    static QByteArray editCss = QByteArray((char*)tss.data(),tss.size());

    auto ret = new DockTitleBar(parent, mw);

    auto ui2 = new Ui_Titlebar();
    ui2->setupUi(ret);
    ret->m_pMain = ui2->m_pMain;
    ret->setStyleSheet(editCss);
    ret->setMinimumSize(0, 30);

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

    /*const bool ret1 = */registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2224")));
    /*const bool ret2 = */registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2225")));

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

    connect(m_pPageModelReplica, &QAbstractItemModel::rowsAboutToBeRemoved, [this](const QModelIndex&, int first, int last) {
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
    static QHash<QString, std::function<QWidget*(QAbstractItemModel*)> > tm {
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

    static QHash<QString, std::function<QWidget*(QObject*)> > to {
        {"scale_node", [this](QObject* obj) {
            auto w = new QWidget(this);
            auto l = new QHBoxLayout(w);
            auto scale = new QwtThermo(w);
            l->addWidget(scale);
            l->addStretch();
            scale->setLowerBound(1);
            scale->setUpperBound(10);
            //lcdw->setModel(model);
            return w;
        }},
    };

    if (tm.contains(wdg.m_Type)) {
        auto w = tm[wdg.m_Type](wdg.m_pModel);
        auto dock = new QDockWidget(wdg.m_ModelName, this);

        dock->setProperty("normalTitlebar", QVariant::fromValue(new QWidget(this)));
        dock->setProperty("editTitlebar", QVariant::fromValue(createTitlebar(dock, this)));
        dock->setTitleBarWidget(qvariant_cast<QWidget*>(
            dock->property("normalTitlebar"))
        );

        dock->setWidget(w);

        addDockWidget(Qt::LeftDockWidgetArea, dock);
        m_lDocks[wdg.m_Uid] = dock;
    }
    else if (to.contains(wdg.m_Type)) {
        auto w = to[wdg.m_Type](wdg.m_pModel);
        auto dock = new QDockWidget(wdg.m_ObjectName, this);

        dock->setProperty("normalTitlebar", QVariant::fromValue(new QWidget(this)));
        dock->setProperty("editTitlebar", QVariant::fromValue(createTitlebar(dock, this)));
        dock->setTitleBarWidget(qvariant_cast<QWidget*>(
            dock->property("normalTitlebar"))
        );

        dock->setWidget(w);

        addDockWidget(Qt::LeftDockWidgetArea, dock);
        m_lDocks[wdg.m_Uid] = dock;
    }
}

void MainWindow::createSection(const QPersistentModelIndex& idx)
{
    if (m_hLoaded[idx])
        return;

    const QString name = idx.data(PageManager::Role::REMOTE_MODEL_NAME  ).toString();
    const QString obj  = idx.data(PageManager::Role::REMOTE_OBJECT_NAME ).toString();
    const QString type = idx.data(PageManager::Role::REMOTE_WIDGET_TYPE ).toString();
    const QString uid  = idx.data(PageManager::Role::REMOTE_OBJECT_UID  ).toString();

    if ((name.isEmpty() && obj.isEmpty()) || type.isEmpty() || uid.isEmpty())
        return;

    auto m = registry->acquireModel(name);


    if (!(m||obj.size()))
        return;

    addWidget({
        type,
        name,
        obj,
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
                dock->setTitleBarWidget(qvariant_cast<QWidget*>(
                    dock->property(edit ? "editTitlebar" : "normalTitlebar"))
                );
            }
        }
    }
}

MainWindow::~MainWindow()
{
    //delete ui;
}

#include <mainwindow.moc>
