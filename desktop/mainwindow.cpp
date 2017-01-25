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

#include "proxynodefactory.h"
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
#include "qt5-node-editor/src/graphicsnodescene.hpp"

#include "common/pagemanager.h"
#include "common/interfaceserializer.h"

static MainWindow* ins; //FIXME

class DesktopSerializer : public InterfaceSerializer
{
public:
    explicit DesktopSerializer(MainWindow* mw) :
        InterfaceSerializer("desktop"), m_pMain(mw) {}
    virtual ~DesktopSerializer() {}

    virtual void reflow() const override;
    virtual void rename(const QString& id, const QString& newName) override;

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

    const auto area = m_pMain->dockWidgetArea(dock);
    const float ratio = (float) ( (float)
            (area & (Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea)) ?
                dock->width() : dock->height()
        ) / (float) ( (float)
            (area & (Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea)) ?
                m_pMain->centralWidget()->width() : m_pMain->centralWidget()->height()
        );

    const int pos = (area & (Qt::DockWidgetArea::TopDockWidgetArea | Qt::DockWidgetArea::BottomDockWidgetArea)) ?
        dock->x() : dock->y();

    parent[ "name"  ] = dock->objectName(); //TODO REMOVE
    parent[ "pos"   ] = pos;
    parent[ "area"  ] = area;
    parent[ "ratio" ] = ratio;
    parent[ "uid"   ] = id;
}

void DesktopSerializer::reflow() const
{
    struct info_t {
        int          pos;
        float        ratio;
        QDockWidget* dock;
    };

    // QMap are ordered, this allows resizeDocks to work
    QMap<int, QMap<int, info_t>> docksByArea;
    //resizeDocks(const QList<QDockWidget *> &docks, const QList<int> &sizes, Qt::Orientation orientation)

    QJsonArray arr;

    // Sort each docks (per area)
    for (const auto e : elements()) {
        const auto  uid = e["uid"].toString();
        Q_ASSERT(!uid.isEmpty());

        if (!m_pMain->m_lDocks.contains(uid))
            continue;

        const int area = e["area"].toInt();

        info_t i {
            e["pos"].toInt(),
            e["ratio"].toDouble(),
            m_pMain->m_lDocks[uid]
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
            m_pMain->addDockWidget(area, i.dock);
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

    m_pMain->m_lDocks[id]->setWindowTitle(newName);
}

QDockWidget* MainWindow::addDock(QWidget* w, const QString& title, const QString& uid)
{
    static QHash<QString, int> uniqueNames;

    auto dock = new QDockWidget(title, this);

    dock->setWidget(w);
    dock->setObjectName(title+QString::number(uniqueNames[title]++));
    addDockWidget(Qt::TopDockWidgetArea, dock);

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

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), fileName(QString())
{
    setObjectName("Master Window");
    ins = this;

    m_pStatusBar = new StatusBar2(this);
    setStatusBar(m_pStatusBar);

    auto w = new QWidget(this);
    setupUi(w);

    m_pNode->setViewport(new QGLWidget(
            QGLFormat(QGL::SampleBuffers)));
    m_pNode->setViewportUpdateMode(
        QGraphicsView::FullViewportUpdate);

    connect(PageManager::instance(), &PageManager::pageAdded, this, &MainWindow::addDock);

    connect(PageManager::instance(), &QAbstractItemModel::rowsAboutToBeRemoved, [this](const QModelIndex& tl, int first, int last) {
        for (int i = first; i <= last; i++) {
            const auto uid = PageManager::instance()->index(i, 0).data(PageManager::Role::REMOTE_OBJECT_UID).toString();

            if (m_lDocks.contains(uid)) {
                removeDockWidget(m_lDocks[uid]);
                m_lDocks.remove(uid);
            }
        }
    });

    m_pSession = new ProxyNodeFactoryAdapter(m_pNode);

    m_pInterfaceSerializer = new DesktopSerializer(this);
    m_pSession->registerInterfaceSerializer(m_pInterfaceSerializer);

    setCentralWidget(w);

    m_pSession->registerType<CurveChartNode>  ("Chart"          , "Widgets"   , "curvedchart_node", QIcon::fromTheme( "document-edit"        ));
    m_pSession->registerType<TableNode>  ("Table"          , "Widgets"   , "table_node", QIcon::fromTheme( "configure-shortcuts"  ));
    m_pSession->registerType<MeterNode>  ("Meter"          , "Widgets"   , "meter_node", QIcon::fromTheme( "bookmark-new"         ));
    m_pSession->registerType<RemoteActionNode>  ("Controls"          , "Widgets"   , "remoteaction_node", QIcon::fromTheme( "bookmark-new"         ));
    m_pSession->registerType<LCDMeterNode>  ("LCD Meter"      , "Widgets"   , "lcdmeter_node", QIcon::fromTheme( "bookmark-new"         ));
    m_pSession->registerType<ColumnNode> ("Range filter"   , "Filters"   , "range_node", QIcon::fromTheme( "view-filter"          ));
    m_pSession->registerType<ColorNode>  ("Range Colorizer", "Metadata"  , "color_node", QIcon::fromTheme( "colors-chromablue"   ));
    m_pSession->registerType<ColumnNode> ("Column filter"  , "Filters"   , "column_node", QIcon::fromTheme( "view-filter"          ));
    m_pSession->registerType<DeduplicateNode> ("Deduplicate"  , "Filters"   , "deduplicate_node", QIcon::fromTheme( "view-filter"          ));
    m_pSession->registerType<AquisitionNode> ("Live aquisition" , "Sources"  , "aquisition_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<ManualAquisitionNode> ("Manual aquisition" , "Sources"  , "manualaquisition_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<MementoNode>("Memento"         , "Sources"  , "memento_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<MultiplexerNode>("Multiplexer" , "Tools"    , "multiplexer_node", QIcon::fromTheme( "edit-copy"          ));
    m_pSession->registerType<TailNode>   ("Tail filter"     , "Filters"  , "tail_node"  , QIcon::fromTheme( "kt-add-filters"   ));
    m_pSession->registerType<HeadNode>   ("Head filter"     , "Filters"  , "head_node" , QIcon::fromTheme( "kt-remove-filters"));
    m_pSession->registerType<CurrentValues>("Current Values", "Sinks"    , "currentvalues_node" , QIcon::fromTheme( "kt-remove-filters"));
    m_pSession->registerType<TimerNode>  ("Timer"           , "Tools"    , "timer_node", QIcon::fromTheme( "chronometer"          ));
    m_pSession->registerType<SequenceNode>  ("Sequence"           , "Tools"    , "sequence_node", QIcon::fromTheme( "chronometer"          ));
    m_pSession->registerType<ChronoNode> ("Chronometer"     , "Metadata" , "chrono_node", QIcon::fromTheme( "chronometer"        ));
    m_pSession->registerType<DeviceListNode> ("Device List"      , "Sources"  , "devicelist_node", QIcon::fromTheme( "document-open"          ));

    m_pSession->registerType<RemoteTable>("Table"         , "Remote widgets"  , "remotetable_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<RemoteMeter>("Meter"         , "Remote widgets"  , "remotemeter_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<RemoteControls>("Controls"      , "Remote widgets"  , "remotecontrols_node", QIcon::fromTheme( "view-calendar-timeline"          ));

    //DUMMY
    m_pSession->registerType<ColorNode> ("File"            , "Sources"  , " ", QIcon::fromTheme( "document-open"          ));
    m_pSession->registerType<ColorNode> ("External device" , "Sources"  , " ", QIcon::fromTheme( "document-share"          )); //allow async memento transfer from other instances
    m_pSession->registerType<ColorNode> ("Statistics"      , "Metadata" , " ", QIcon::fromTheme( "format-number-percent"        ));
    m_pSession->registerType<ColorNode> ("CSV"             , "Exporter" , " ", QIcon::fromTheme( "document-save"      ));
    m_pSession->registerType<ColorNode> ("XLSX"            , "Exporter" , " ", QIcon::fromTheme( "document-share"     ));
    m_pSession->registerType<ColorNode> ("ODS"             , "Exporter" , " ", QIcon::fromTheme( "document-save-all"  ));
    m_pSession->registerType<ColorNode> ("PCAP (WireShark)", "Exporter" , " ", QIcon::fromTheme( "document-save-as"   ));

    m_pSession->registerType<ColorNode> ("Rate watchdog"   , "Sinks"    , "" , QIcon::fromTheme( "mail-forward"     ));
    m_pSession->registerType<ColorNode> ("Unit filter"     , "Filters"  , "" , QIcon::fromTheme( "kt-remove-filters"));

    //Create the node creator dock
    auto dock = new QDockWidget    ( ins );
    auto tab  = new CategorizedTree( dock     );
    dock->setObjectName("ToolBox");
    tab->setModel(m_pSession);
    tab->setDragEnabled(true);
    tab->setDragDropMode(QAbstractItemView::DragOnly);
    auto del = new CategorizedDelegate(tab);
    del->setChildDelegate(new AutoCompletionDelegate());
    tab->setItemDelegate(del);
    tab->setIndentation(5);
    tab->expandAll();
    QObject::connect(tab, &QTreeView::doubleClicked, [this](const QModelIndex& idx) {
        m_pSession->addToScene(idx);
    });

    dock->setWidget(tab);
    ins->addDockWidget(Qt::LeftDockWidgetArea, dock);

    setupActions();

    if (Settings::openLastFile() && !Settings::lastFilePath().isEmpty())
        openFile(Settings::lastFilePath());
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
        QSaveFile file(outputFileName.path());
        file.open(QIODevice::WriteOnly);

        m_pSession->serialize(&file);
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

    m_pSession->load(storedJob->data());
}
