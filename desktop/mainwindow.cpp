#include <QApplication>
#include <QAction>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>

#include <KLocalizedString>
#include <KActionCollection>
#include <KStandardAction>
#include <KMessageBox>
#include <KConfigDialog>
#include <KIO/Job>

#include <QtOpenGL/QGLWidget>

#include "sigrokd/aquisitionmodel.h"

#include "docktitle.h"

#include "session.h"
#include "models/remotewidgets.h"

#include "nodes/aquisitionnode.h"
#include "nodes/manualaquisitionnode.h"
// #include "nodes/chartnode.h"
#include "nodes/curvechartnode.h"
#include "nodes/tablenode.h"
#include "nodes/meternode.h"
#include "nodes/lcdmeternode.h"
#include "nodes/columnnode.h"
#include "nodes/colornode.h"
#include "nodes/mementonode.h"
#include "nodes/tailnode.h"
#include "nodes/headnode.h"
#include "nodes/timernode.h"
#include "nodes/chrononode.h"
#include "nodes/sequencenode.h"
#include "nodes/currentvalues.h"
#include "nodes/remoteaction.h"
#include "nodes/devicelistnode.h"
#include "nodes/multiplexernode.h"
#include "nodes/remoteaction.h"
#include "nodes/deduplicatenode.h"
#include "nodes/remote/remotetable.h"
#include "nodes/remote/remotemeter.h"
#include "nodes/remote/remotecontrols.h"

#include "widgets/devicelist.h"
#include "widgets/charttype.h"
#include "widgets/categorizedtree.h"
#include "widgets/statusbar.h"
#include "configdialog.h"

#include "delegates/categorizeddelegate.h"
#include "delegates/autocompletiondelegate.h"

#include <QtCore/QDebug>

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QTreeView>

#include "mainwindow.h"

#include "tutorial4Settings.h"

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/qnodewidget.h"
#include "qt5-node-editor/src/graphicsnodescene.hpp"

#include "common/pagemanager.h"
#include "common/interfaceserializer.h"
#include "common/widgetgroupmodel.h"

static MainWindow* ins; //FIXME

class DesktopSerializer : public InterfaceSerializer
{
public:
    explicit DesktopSerializer(MainWindow* mw) :
        InterfaceSerializer("desktop"), m_pMain(mw) {}
    virtual ~DesktopSerializer() {}

    virtual void reflow() const override;
    virtual void rename(const QString& id, const QString& newName) override;

    QMainWindow* getWindowWinId(const QString& uid) const;

protected:
    virtual void writeWidget(QJsonObject &parent, const QString& id) const override;

private:
    MainWindow* m_pMain;
};

void DesktopSerializer::writeWidget(QJsonObject &parent, const QString& id) const
{
    QDockWidget* dock = m_pMain->m_lDocks[id];

    if (Q_UNLIKELY(!dock)) {
        qWarning() << "Trying to save a widget that doesn't exist";
        return;
    }

    auto mw = getWindowWinId(id);
    Q_ASSERT(mw);

    const auto area = mw->dockWidgetArea(dock);
    const float ratio = (float) ( (float)
            (area & (Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea)) ?
                dock->width() : dock->height()
        ) / (float) ( (float)
            (area & (Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea)) ?
                mw->centralWidget()->width() : mw->centralWidget()->height()
        );

    const int pos = (area & (Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea)) ?
        dock->x() : dock->y();

    const auto winId = dock->parentWidget() ?
        dock->parentWidget()->objectName() : "floating";

    Q_ASSERT(!winId.isEmpty());

    parent[ "name"   ] = dock->objectName(); //TODO REMOVE
    parent[ "pos"    ] = pos;
    parent[ "area"   ] = area;
    parent[ "ratio"  ] = ratio;
    parent[ "uid"    ] = id;
    parent[ "window" ] = winId;
}

QMainWindow* DesktopSerializer::getWindowWinId(const QString& uid) const
{
    QString windowId;

    for (const auto e : elements()) {
        if (e["uid"].toString() == uid) {
            windowId = e["window"].toString();
            break;
        }
    }

    if (m_pMain->m_lWindows.contains(windowId))
        return m_pMain->m_lWindows[windowId];

    if (!windowId.isEmpty())
        return m_pMain->addMainWindow(QObject::tr("Window"), windowId);

    Q_ASSERT(m_pMain->m_lWindows.contains("mainWindow"));

    return m_pMain->m_lWindows["mainWindow"];
}

void DesktopSerializer::reflow() const
{
    struct info_t {
        int          pos;
        float        ratio;
        QDockWidget* dock;
        QMainWindow* mw;
    };

    // QMap are ordered, this allows resizeDocks to work
    QMap<int, QMap<int, info_t>> docksByArea;

    // Sort each docks (per area)
    for (const auto e : elements()) {
        const auto  uid = e["uid"].toString();
        Q_ASSERT(!uid.isEmpty());

        if (!m_pMain->m_lDocks.contains(uid))
            continue;

        const int area = e["area"].toInt();

        const auto winId = e["area"].toString();

        auto mw = ((!winId.isEmpty()) && m_pMain->m_lWindows.contains(winId)) ?
            m_pMain->m_lWindows[winId] : getWindowWinId(uid);

        info_t i {
            e[ "pos"   ].toInt(),
            e[ "ratio" ].toDouble(),
            m_pMain->m_lDocks[uid],
            mw
        };

        Q_ASSERT(dynamic_cast<QDockWidget*>(i.dock));

        docksByArea[area][i.pos] = i;
    }

    typedef Qt::DockWidgetArea DWA;

    // Place the docks
    for (auto area : {
        DWA::TopDockWidgetArea , DWA::BottomDockWidgetArea,
        DWA::LeftDockWidgetArea, DWA::RightDockWidgetArea
    }) {
        //TODO check if its present
        const int total = area > 1 ?
            m_pMain->centralWidget()->height() :m_pMain->centralWidget()->width();

        QList<QDockWidget*> docks;
        QList<int> sizes;

        for (const auto& i : docksByArea[area]) {
            i.mw->addDockWidget(area, i.dock);
            docks << i.dock;
            sizes << (i.ratio*total);
        }
        m_pMain->resizeDocks(docks, sizes, Qt::Horizontal);
    }

}

void DesktopSerializer::rename(const QString& id, const QString& newName)
{
    if (!m_pMain->m_lDocks.contains(id))
        return;

    m_pMain->m_lDocks[id]->setText(newName);
}

QDockWidget* MainWindow::addDock(QWidget* w, const QString& title, const QString& uid)
{
    static QHash<QString, int> uniqueNames;

    auto mw = m_pInterfaceSerializer->getWindowWinId(uid);
    Q_ASSERT(mw);

    auto dock = new DockTitle(mw);
    dock->setModel(m_pGroups);
    dock->setText(title);

    dock->setWidget(w);
    dock->setObjectName(title+QString::number(uniqueNames[title]++));
    qDebug() << mw->objectName();
    mw->addDockWidget(Qt::TopDockWidgetArea, dock);

    if (Q_UNLIKELY(m_lDocks.contains(uid)))
        qWarning() << "A Widget with the same ID already exist" << uid;

    m_lDocks[uid] = dock;
    m_pInterfaceSerializer->add(uid);
    m_pInterfaceSerializer->reflow();


    connect(dock, &QObject::destroyed, [this, uid] {
        m_lDocks.remove(uid);
    });

    return dock;
}

Session* MainWindow::addSession(const QString& name)
{
    auto nodeWidget = new QNodeWidget(this->m_pTabs);
    auto sess = new Session(nodeWidget);
    m_lSessions << sess;
    m_pTabs->addTab(nodeWidget, name);


    connect(sess->pages(), &PageManager::pageAdded, this, &MainWindow::addDock);

    connect(sess->pages(), &QAbstractItemModel::rowsAboutToBeRemoved, [this, sess](const QModelIndex& tl, int first, int last) {
        for (int i = first; i <= last; i++) {
            const auto uid = sess->pages()->index(i, 0).data(PageManager::Role::REMOTE_OBJECT_UID).toString();

            if (m_lDocks.contains(uid)) {
                removeDockWidget(m_lDocks[uid]);
                m_lDocks.remove(uid);
            }
        }
    });

    m_pGroups = new WidgetGroupModel(this);
    m_pGroups->addGroup(this, tr("Main"), "mainWindow");

    nodeWidget->setViewport(new QGLWidget(
            QGLFormat(QGL::SampleBuffers)));
    nodeWidget->setViewportUpdateMode(
        QGraphicsView::FullViewportUpdate);


    m_pInterfaceSerializer = new DesktopSerializer(this);
    sess->registerInterfaceSerializer(m_pInterfaceSerializer);

    sess->registerType<CurveChartNode>  ("Chart"          , "Widgets"   , "curvedchart_node", QIcon::fromTheme( "document-edit"        ));
    sess->registerType<TableNode>  ("Table"          , "Widgets"   , "table_node", QIcon::fromTheme( "configure-shortcuts"  ));
    sess->registerType<MeterNode>  ("Meter"          , "Widgets"   , "meter_node", QIcon::fromTheme( "bookmark-new"         ));
    sess->registerType<RemoteActionNode>  ("Controls"          , "Widgets"   , "remoteaction_node", QIcon::fromTheme( "bookmark-new"         ));
    sess->registerType<LCDMeterNode>  ("LCD Meter"      , "Widgets"   , "lcdmeter_node", QIcon::fromTheme( "bookmark-new"         ));
    sess->registerType<ColumnNode> ("Range filter"   , "Filters"   , "range_node", QIcon::fromTheme( "view-filter"          ));
    sess->registerType<ColorNode>  ("Range Colorizer", "Metadata"  , "color_node", QIcon::fromTheme( "colors-chromablue"   ));
    sess->registerType<ColumnNode> ("Column filter"  , "Filters"   , "column_node", QIcon::fromTheme( "view-filter"          ));
    sess->registerType<DeduplicateNode> ("Deduplicate"  , "Filters"   , "deduplicate_node", QIcon::fromTheme( "view-filter"          ));
    sess->registerType<AquisitionNode> ("Live aquisition" , "Sources"  , "aquisition_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    sess->registerType<ManualAquisitionNode> ("Manual aquisition" , "Sources"  , "manualaquisition_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    sess->registerType<MementoNode>("Memento"         , "Sources"  , "memento_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    sess->registerType<MultiplexerNode>("Multiplexer" , "Tools"    , "multiplexer_node", QIcon::fromTheme( "edit-copy"          ));
    sess->registerType<TailNode>   ("Tail filter"     , "Filters"  , "tail_node"  , QIcon::fromTheme( "kt-add-filters"   ));
    sess->registerType<HeadNode>   ("Head filter"     , "Filters"  , "head_node" , QIcon::fromTheme( "kt-remove-filters"));
    sess->registerType<CurrentValues>("Current Values", "Sinks"    , "currentvalues_node" , QIcon::fromTheme( "kt-remove-filters"));
    sess->registerType<TimerNode>  ("Timer"           , "Tools"    , "timer_node", QIcon::fromTheme( "chronometer"          ));
    sess->registerType<SequenceNode>  ("Sequence"           , "Tools"    , "sequence_node", QIcon::fromTheme( "chronometer"          ));
    sess->registerType<ChronoNode> ("Chronometer"     , "Metadata" , "chrono_node", QIcon::fromTheme( "chronometer"        ));
    sess->registerType<DeviceListNode> ("Device List"      , "Sources"  , "devicelist_node", QIcon::fromTheme( "document-open"          ));

    sess->registerType<RemoteTable>("Table"         , "Remote widgets"  , "remotetable_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    sess->registerType<RemoteMeter>("Meter"         , "Remote widgets"  , "remotemeter_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    sess->registerType<RemoteControls>("Controls"      , "Remote widgets"  , "remotecontrols_node", QIcon::fromTheme( "view-calendar-timeline"          ));

    //DUMMY
    sess->registerType<ColorNode> ("File"            , "Sources"  , " ", QIcon::fromTheme( "document-open"          ));
    sess->registerType<ColorNode> ("External device" , "Sources"  , " ", QIcon::fromTheme( "document-share"          )); //allow async memento transfer from other instances
    sess->registerType<ColorNode> ("Statistics"      , "Metadata" , " ", QIcon::fromTheme( "format-number-percent"        ));
    sess->registerType<ColorNode> ("CSV"             , "Exporter" , " ", QIcon::fromTheme( "document-save"      ));
    sess->registerType<ColorNode> ("XLSX"            , "Exporter" , " ", QIcon::fromTheme( "document-share"     ));
    sess->registerType<ColorNode> ("ODS"             , "Exporter" , " ", QIcon::fromTheme( "document-save-all"  ));
    sess->registerType<ColorNode> ("PCAP (WireShark)", "Exporter" , " ", QIcon::fromTheme( "document-save-as"   ));

    sess->registerType<ColorNode> ("Rate watchdog"   , "Sinks"    , "" , QIcon::fromTheme( "mail-forward"     ));
    sess->registerType<ColorNode> ("Unit filter"     , "Filters"  , "" , QIcon::fromTheme( "kt-remove-filters"));

    return sess;
}

Session* MainWindow::currentSession() const
{
    return m_lSessions[m_pTabs->currentIndex()];
}

void MainWindow::slotTabCloseRequested(int index)
{
    qDebug() << "CLOSE REQUESTED!!!";
}

void MainWindow::slotTabSelected(int index)
{
    auto sess = m_lSessions[index];
    qDebug() << "CURRET CHANGED" << index;
    m_pToolBox->setModel(sess);
    m_pToolBox->expandAll();

    if (!sess->rowCount())
        QTimer::singleShot(0, m_pToolBox, &QTreeView::expandAll);
}

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), fileName(QString())
{
    setObjectName("mainWindow");
    m_lWindows["mainWindow"] = this;
    ins = this;

    m_pStatusBar = new StatusBar2(this);
    setStatusBar(m_pStatusBar);

    auto w = new QWidget(this);
    setupUi(w);

    connect(m_pTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::slotTabCloseRequested);
    connect(m_pTabs, &QTabWidget::currentChanged, this, &MainWindow::slotTabSelected);

    setCentralWidget(w);

    //Create the node creator dock
    auto dock = new QDockWidget( "Tool box", ins );
    m_pToolBox = new CategorizedTree(dock);
    m_pToolBox->header()->setHidden(true);
    dock->setObjectName("ToolBox");
    m_pToolBox->setDragEnabled(true);
    m_pToolBox->setDragDropMode(QAbstractItemView::DragOnly);
    auto del = new CategorizedDelegate(m_pToolBox);
    del->setChildDelegate(new AutoCompletionDelegate());
    m_pToolBox->setItemDelegate(del);
    m_pToolBox->setIndentation(5);
    QObject::connect(m_pToolBox, &QTreeView::doubleClicked, [this](const QModelIndex& idx) {
        const int cur = m_pTabs->currentIndex();
        Q_ASSERT(cur >= 0 && cur < m_lSessions.size());

        m_lSessions[cur]->addToScene(idx);
    });

    dock->setWidget(m_pToolBox);
    ins->addDockWidget(Qt::LeftDockWidgetArea, dock);

    setupActions();


    if (Settings::openLastFile() && !Settings::lastFilePath().isEmpty())
        openFile(Settings::lastFilePath());
    else
        addSession("dsfsdf");

}

MainWindow::~MainWindow()
{
//     auto dock, m_lDocks
}

MainWindow* MainWindow::instance()
{
    return ins;
}

void MainWindow::setupActions()
{
    QAction* clearAction = new QAction(this);
    clearAction->setText(i18n("&Clear"));
    clearAction->setIcon(QIcon::fromTheme("document-new"));
    actionCollection()->setDefaultShortcut(clearAction, Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction("clear", clearAction);

    QAction* addWindowAction = new QAction(this);
    addWindowAction->setText(i18n("&Add window"));
    addWindowAction->setIcon(QIcon::fromTheme("document-new"));
    actionCollection()->setDefaultShortcut(addWindowAction, Qt::CTRL + Qt::Key_A);
    actionCollection()->addAction("add_window", addWindowAction);
    connect(addWindowAction, &QAction::triggered, this, &MainWindow::slotAddMainWindow);

    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    KStandardAction::save(this, SLOT(saveFile()), actionCollection());

    KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());

    KStandardAction::openNew(this, SLOT(newFile()), actionCollection());

    KStandardAction::preferences(this, SLOT(settingsConfigure()), actionCollection());

    setupGUI(Default, "tutorial4ui.rc");
}

void MainWindow::newFile()
{
    fileName.clear();
}

void MainWindow::saveFileAs(const QUrl &outputFileName)
{
    if (!outputFileName.isEmpty()) {
        auto sess = currentSession();
        Q_ASSERT(sess);

        QSaveFile file(outputFileName.path());
        file.open(QIODevice::WriteOnly);

        sess->serialize(&file);
        file.commit();

        fileName = outputFileName;

        Settings::setLastFilePath(QUrl(outputFileName));
    }
}

void MainWindow::saveFileAs()
{
    saveFileAs(QFileDialog::getSaveFileUrl(this, i18n("Save File As")));
}

void MainWindow::saveFile()
{
    if (!fileName.isEmpty()) {
        saveFileAs(fileName);
    }
    else {
        saveFileAs();
    }
}

void MainWindow::openFile(const QUrl &name)
{
    if (!name.isEmpty()) {
        KIO::Job* job = KIO::storedGet(name);
        fileName = name;

        connect(job, SIGNAL(result(KJob*)), this, SLOT(downloadFinished(KJob*)));

        job->exec();
    }
}

void MainWindow::openFile()
{
    QUrl fileNameFromDialog = QFileDialog::getOpenFileUrl(this, i18n("Open File"));
    openFile(fileNameFromDialog);
}

void MainWindow::settingsConfigure()
{
    if (KConfigDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }
    ConfigDialog *dialog = new ConfigDialog(this);

    dialog->show();
}

void MainWindow::downloadFinished(KJob* job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorString());
        fileName.clear();
        return;
    }

    KIO::StoredTransferJob* storedJob = (KIO::StoredTransferJob*)job;

    auto sess = addSession("fo");

    sess->load(storedJob->data());
}

QMainWindow* MainWindow::slotAddMainWindow(bool)
{
    return addMainWindow(tr("Window"));
}

QMainWindow* MainWindow::addMainWindow(const QString& title, const QString& id)
{
    QMainWindow* w = new QMainWindow(this);
    auto wdg = new QWidget();
    wdg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    w->setCentralWidget(new QWidget());
    QDockWidget* d = new QDockWidget();
    const auto id2 = m_pGroups->addGroup(w, title, id.isEmpty() ? title : id);
    w->setObjectName(id2);
    w->show();

    Q_ASSERT(id.isEmpty() || id == id2);

    m_lWindows[id2] = w;

    return w;
}

#include <mainwindow.moc>
