#include <QApplication>
#include <QAction>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>

#include <QtCore/QJsonDocument>
#include <QtCore/QMimeData>
#include <QtGui/QClipboard>
#include <QtGui/QGuiApplication>
#include <QtGui/QMouseEvent>

#include <QtWidgets/QMenu>

#include <KLocalizedString>
#include <KActionCollection>
#include <KStandardAction>
#include <KMessageBox>
#include <KConfigDialog>
#include <KIO/Job>

#include <QtOpenGL/QGLWidget>

#include "sigrokd/aquisitionmodel.h"

#include "docktitle.h"
#include "toolbox.h"


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

#include <QtCore/QDebug>

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QTreeView>

#include "mainwindow.h"

#include "tutorial4Settings.h"

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/qnodewidget.h"
#include "qt5-node-editor/src/graphicsnodescene.hpp"

#include "Qt-Color-Widgets/include/ColorDialog"

#include "common/pagemanager.h"
#include "common/interfaceserializer.h"
#include "common/widgetgroupmodel.h"
#include "common/actioncollection.h"
#include "common/selectedactioncollection.h"

#include "qrc_desktop.cpp"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class DesktopSerializer : public InterfaceSerializer
{
public:
    explicit DesktopSerializer(MainWindow* mw) :
        InterfaceSerializer(QStringLiteral("desktop")), m_pMain(mw) {}
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
        dock->parentWidget()->objectName() : QStringLiteral("floating");

    Q_ASSERT(!winId.isEmpty());

    parent[ QStringLiteral("name")   ] = dock->objectName(); //TODO REMOVE
    parent[ QStringLiteral("pos")    ] = pos;
    parent[ QStringLiteral("area")   ] = area;
    parent[ QStringLiteral("ratio")  ] = ratio;
    parent[ QStringLiteral("uid")    ] = id;
    parent[ QStringLiteral("window") ] = winId;
}

QMainWindow* DesktopSerializer::getWindowWinId(const QString& uid) const
{
    QString windowId;

    for (const auto& e : qAsConst(elements())) {
        if (e[QStringLiteral("uid")].toString() == uid) {
            windowId = e[QStringLiteral("window")].toString();
            break;
        }
    }

    if (m_pMain->m_lWindows.contains(windowId))
        return m_pMain->m_lWindows[windowId];

    if (!windowId.isEmpty())
        return m_pMain->addMainWindow(QObject::tr("Window"), windowId);

    Q_ASSERT(m_pMain->m_lWindows.contains(QStringLiteral("mainWindow")));

    return m_pMain->m_lWindows[QStringLiteral("mainWindow")];
}

void DesktopSerializer::reflow() const
{
    struct info_t {
        int          pos;
        double       ratio;
        QDockWidget* dock;
        QMainWindow* mw;
    };

    // QMap are ordered, this allows resizeDocks to work
    QMap<int, QMap<int, info_t>> docksByArea;

    // Sort each docks (per area)
    for (const auto& e : qAsConst(elements())) {
        const auto  uid = e[QStringLiteral("uid")].toString();
        Q_ASSERT(!uid.isEmpty());

        if (!m_pMain->m_lDocks.contains(uid))
            continue;

        const int area = e[QStringLiteral("area")].toInt();

        const auto winId = e[QStringLiteral("area")].toString();

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

    QList<QDockWidget*> docks;
    QList<int> sizes;

    // Place the docks
    for (auto area : {
        DWA::TopDockWidgetArea , DWA::BottomDockWidgetArea,
        DWA::LeftDockWidgetArea, DWA::RightDockWidgetArea
    }) {
        //TODO check if its present
        const int total = area > 1 ?
            m_pMain->centralWidget()->height() :m_pMain->centralWidget()->width();

        for (const auto& i : qAsConst(docksByArea[area])) {
            i.mw->addDockWidget(area, i.dock);
            docks << i.dock;
            sizes << (i.ratio*total);
        }

        m_pMain->resizeDocks(docks, sizes, Qt::Horizontal);

        docks.clear();
        sizes.clear();
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
    connect(nodeWidget, &QNodeWidget::currentNodeChanged, this, &MainWindow::slotSelectionChanged);
    connect(nodeWidget, &QNodeWidget::zoomLevelChanged, this, &MainWindow::zoomLevelChanged);

    connect(nodeWidget, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(slotNodeContextMenu(QPoint)));

    nodeWidget->installEventFilter(this);

    connect(sess->pages(), &QAbstractItemModel::rowsAboutToBeRemoved, [this, sess](const QModelIndex& , int first, int last) {
        for (int i = first; i <= last; i++) {
            const auto uid = sess->pages()->index(i, 0).data(PageManager::Role::REMOTE_OBJECT_UID).toString();

            if (m_lDocks.contains(uid)) {
                removeDockWidget(m_lDocks[uid]);
                m_lDocks.remove(uid);
            }
        }
    });

    m_pGroups = new WidgetGroupModel(this);
    m_pGroups->addGroup(this, tr("Main"), QStringLiteral("mainWindow"));

    nodeWidget->setViewport(new QGLWidget(
            QGLFormat(QGL::SampleBuffers)));
    nodeWidget->setViewportUpdateMode(
        QGraphicsView::FullViewportUpdate);


    m_pInterfaceSerializer = new DesktopSerializer(this);
    sess->registerInterfaceSerializer(m_pInterfaceSerializer);

    sess->registerType<CurveChartNode>  (QStringLiteral("Chart")          , QStringLiteral("Widgets")   , QStringLiteral("curvedchart_node"), QIcon::fromTheme( QStringLiteral("document-edit")        ));
    sess->registerType<TableNode>  (QStringLiteral("Table")          , QStringLiteral("Widgets")   , QStringLiteral("table_node"), QIcon::fromTheme( QStringLiteral("configure-shortcuts")  ));
    sess->registerType<MeterNode>  (QStringLiteral("Meter")          , QStringLiteral("Widgets")   , QStringLiteral("meter_node"), QIcon::fromTheme( QStringLiteral("bookmark-new")         ));
    sess->registerType<RemoteActionNode>  (QStringLiteral("Controls")          , QStringLiteral("Widgets")   , QStringLiteral("remoteaction_node"), QIcon::fromTheme( QStringLiteral("bookmark-new")         ));
    sess->registerType<LCDMeterNode>  (QStringLiteral("LCD Meter")      , QStringLiteral("Widgets")   , QStringLiteral("lcdmeter_node"), QIcon::fromTheme( QStringLiteral("bookmark-new")         ));
    sess->registerType<ColumnNode> (QStringLiteral("Range filter")   , QStringLiteral("Filters")   , QStringLiteral("range_node"), QIcon::fromTheme( QStringLiteral("view-filter")          ));
    sess->registerType<ColorNode>  (QStringLiteral("Range Colorizer"), QStringLiteral("Metadata")  , QStringLiteral("color_node"), QIcon::fromTheme( QStringLiteral("colors-chromablue")   ));
    sess->registerType<ColumnNode> (QStringLiteral("Column filter")  , QStringLiteral("Filters")   , QStringLiteral("column_node"), QIcon::fromTheme( QStringLiteral("view-filter")          ));
    sess->registerType<DeduplicateNode> (QStringLiteral("Deduplicate")  , QStringLiteral("Filters")   , QStringLiteral("deduplicate_node"), QIcon::fromTheme( QStringLiteral("view-filter")          ));
    sess->registerType<AquisitionNode> (QStringLiteral("Live aquisition") , QStringLiteral("Sources")  , QStringLiteral("aquisition_node"), QIcon::fromTheme( QStringLiteral("view-calendar-timeline")          ));
    sess->registerType<ManualAquisitionNode> (QStringLiteral("Manual aquisition") , QStringLiteral("Sources")  , QStringLiteral("manualaquisition_node"), QIcon::fromTheme( QStringLiteral("view-calendar-timeline")          ));
    sess->registerType<MementoNode>(QStringLiteral("Memento")         , QStringLiteral("Sources")  , QStringLiteral("memento_node"), QIcon::fromTheme( QStringLiteral("view-calendar-timeline")          ));
    sess->registerType<MultiplexerNode>(QStringLiteral("Multiplexer") , QStringLiteral("Tools")    , QStringLiteral("multiplexer_node"), QIcon::fromTheme( QStringLiteral("edit-copy")          ));
    sess->registerType<TailNode>   (QStringLiteral("Tail filter")     , QStringLiteral("Filters")  , QStringLiteral("tail_node")  , QIcon::fromTheme( QStringLiteral("kt-add-filters")   ));
    sess->registerType<HeadNode>   (QStringLiteral("Head filter")     , QStringLiteral("Filters")  , QStringLiteral("head_node") , QIcon::fromTheme( QStringLiteral("kt-remove-filters")));
    sess->registerType<CurrentValues>(QStringLiteral("Current Values"), QStringLiteral("Sinks")    , QStringLiteral("currentvalues_node") , QIcon::fromTheme( QStringLiteral("kt-remove-filters")));
    sess->registerType<TimerNode>  (QStringLiteral("Timer")           , QStringLiteral("Tools")    , QStringLiteral("timer_node"), QIcon::fromTheme( QStringLiteral("chronometer")          ));
    sess->registerType<SequenceNode>  (QStringLiteral("Sequence")           , QStringLiteral("Tools")    , QStringLiteral("sequence_node"), QIcon::fromTheme( QStringLiteral("chronometer")          ));
    sess->registerType<ChronoNode> (QStringLiteral("Chronometer")     , QStringLiteral("Metadata") , QStringLiteral("chrono_node"), QIcon::fromTheme( QStringLiteral("chronometer")        ));
    sess->registerType<DeviceListNode> (QStringLiteral("Device List")      , QStringLiteral("Sources")  , QStringLiteral("devicelist_node"), QIcon::fromTheme( QStringLiteral("document-open")          ));

    sess->registerType<RemoteTable>(QStringLiteral("Table")         , QStringLiteral("Remote widgets")  , QStringLiteral("remotetable_node"), QIcon::fromTheme( QStringLiteral("view-calendar-timeline")          ));
    sess->registerType<RemoteMeter>(QStringLiteral("Meter")         , QStringLiteral("Remote widgets")  , QStringLiteral("remotemeter_node"), QIcon::fromTheme( QStringLiteral("view-calendar-timeline")          ));
    sess->registerType<RemoteControls>(QStringLiteral("Controls")      , QStringLiteral("Remote widgets")  , QStringLiteral("remotecontrols_node"), QIcon::fromTheme( QStringLiteral("view-calendar-timeline")          ));

    //DUMMY
    sess->registerType<ColorNode> (QStringLiteral("File")            , QStringLiteral("Sources")  , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("document-open")          ));
    sess->registerType<ColorNode> (QStringLiteral("External device") , QStringLiteral("Sources")  , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("document-share")          )); //allow async memento transfer from other instances
    sess->registerType<ColorNode> (QStringLiteral("Statistics")      , QStringLiteral("Metadata") , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("format-number-percent")        ));
    sess->registerType<ColorNode> (QStringLiteral("CSV")             , QStringLiteral("Exporter") , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("document-save")      ));
    sess->registerType<ColorNode> (QStringLiteral("XLSX")            , QStringLiteral("Exporter") , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("document-share")     ));
    sess->registerType<ColorNode> (QStringLiteral("ODS")             , QStringLiteral("Exporter") , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("document-save-all")  ));
    sess->registerType<ColorNode> (QStringLiteral("PCAP (WireShark)"), QStringLiteral("Exporter") , QStringLiteral(" "), QIcon::fromTheme( QStringLiteral("document-save-as")   ));

    sess->registerType<ColorNode> (QStringLiteral("Rate watchdog")   , QStringLiteral("Sinks")    , QLatin1String("") , QIcon::fromTheme( QStringLiteral("mail-forward")     ));
    sess->registerType<ColorNode> (QStringLiteral("Unit filter")     , QStringLiteral("Filters")  , QLatin1String("") , QIcon::fromTheme( QStringLiteral("kt-remove-filters")));

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

    m_pSelActionCol->sessionChanged(sess);

    if (!sess->rowCount())
        QTimer::singleShot(0, m_pToolBox, &ToolBox::expandAll);
}

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), fileName(),
    m_pActionCollection(new ActionCollection(this)),
    m_pSelActionCol(new SelectedActionCollection(this))
{
    setObjectName(QStringLiteral("mainWindow"));
    m_lWindows[QStringLiteral("mainWindow")] = this;

    static QResource ss(QStringLiteral(":/pref/tutorial4ui.rc"));
    Q_ASSERT(ss.isValid());

    m_pStatusBar = new StatusBar2(this);
    setStatusBar(m_pStatusBar);
    connect(m_pStatusBar, &StatusBar2::zoomLevel, this, &MainWindow::setZoom);

    auto w = new QWidget(this);
    setupUi(w);

    connect(m_pTabs, &QTabWidget::tabCloseRequested, this, &MainWindow::slotTabCloseRequested);
    connect(m_pTabs, &QTabWidget::currentChanged, this, &MainWindow::slotTabSelected);

    connect(m_pActionCollection, &ActionCollection::zoomIn, this, &MainWindow::zoomIn);
    connect(m_pActionCollection, &ActionCollection::zoomOut, this, &MainWindow::zoomOut);
    connect(m_pActionCollection, &ActionCollection::zoomFit, this, &MainWindow::zoomFit);
    connect(m_pActionCollection, &ActionCollection::zoomReset, this, &MainWindow::zoomReset);
    connect(m_pActionCollection, &ActionCollection::paste, this, &MainWindow::slotPaste);

    connect(m_pSelActionCol->bg(), &QAction::triggered, this, &MainWindow::slotBg);
    connect(m_pSelActionCol->fg(), &QAction::triggered, this, &MainWindow::slotFg);

    setCentralWidget(w);

    //Create the node toolbox dock
    m_pToolBox = new ToolBox(this);

    QObject::connect(m_pToolBox, &ToolBox::doubleClicked, [this](const QModelIndex& idx) {
        const int cur = m_pTabs->currentIndex();
        Q_ASSERT(cur >= 0 && cur < m_lSessions.size());

        m_lSessions[cur]->addToScene(idx);
    });

    addDockWidget(Qt::LeftDockWidgetArea, m_pToolBox);

    setupActions();


    if (Settings::openLastFile() && !Settings::lastFilePath().isEmpty())
        openFile(Settings::lastFilePath());
    else
        addSession(QStringLiteral("dsfsdf"));

}

MainWindow::~MainWindow()
{
}


void MainWindow::setupActions()
{
    QAction* clearAction = new QAction(this);
    clearAction->setText(i18n("&Clear"));
    clearAction->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    actionCollection()->setDefaultShortcut(clearAction, Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction(QStringLiteral("clear"), clearAction);

    QAction* addWindowAction = new QAction(this);
    addWindowAction->setText(i18n("&Add window"));
    addWindowAction->setIcon(QIcon::fromTheme(QStringLiteral("document-new")));
    actionCollection()->setDefaultShortcut(addWindowAction, Qt::CTRL + Qt::Key_A);
    actionCollection()->addAction(QStringLiteral("add_window"), addWindowAction);
    connect(addWindowAction, &QAction::triggered, this, &MainWindow::slotAddMainWindow);

    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    KStandardAction::save(this, SLOT(saveFile()), actionCollection());

    KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());

    KStandardAction::openNew(this, SLOT(newFile()), actionCollection());

    KStandardAction::preferences(this, SLOT(settingsConfigure()), actionCollection());

    setupGUI(Default, QStringLiteral(":/pref/tutorial4ui.rc"));
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

        connect(job, &KJob::result, this, &MainWindow::downloadFinished);

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

    auto sess = addSession(QStringLiteral("fo"));

    sess->load(storedJob->data());
}

QMainWindow* MainWindow::slotAddMainWindow(bool)
{
    return addMainWindow(tr("Window"));
}

QNodeWidget* MainWindow::currentNodeWidget() const
{
    auto w = m_pTabs->widget(m_pTabs->currentIndex());

    return qobject_cast<QNodeWidget*>(w);
}

QMainWindow* MainWindow::addMainWindow(const QString& title, const QString& id)
{
    QMainWindow* w = new QMainWindow(this);
    auto wdg = new QWidget();
    wdg->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    w->setCentralWidget(new QWidget());

    const auto id2 = m_pGroups->addGroup(w, title, id.isEmpty() ? title : id);
    w->setObjectName(id2);
    w->show();

    Q_ASSERT(id.isEmpty() || id == id2);

    m_lWindows[id2] = w;

    return w;
}

QMainWindow* MainWindow::slotSelectionChanged()
{
    if (!m_pTabs->count())
        return nullptr;

    if (auto w = currentNodeWidget())
        m_pSelActionCol->currentChanged(w->currentNode());

    return nullptr;
}

void MainWindow::zoomIn()
{
    if (auto w = currentNodeWidget())
        w->setZoomLevel(w->zoomLevel() + 0.5);
}

void MainWindow::zoomOut()
{
    if (auto w = currentNodeWidget())
        w->setZoomLevel(w->zoomLevel() - 0.5);
}

void MainWindow::zoomFit()
{

}

void MainWindow::zoomReset()
{
    if (auto w = currentNodeWidget())
        w->setZoomLevel(1);
}

void MainWindow::setZoom(qreal level)
{
    if (auto w = currentNodeWidget())
        w->setZoomLevel(level);
}

void MainWindow::zoomLevelChanged(qreal level)
{
    if (sender() == currentNodeWidget())
        m_pStatusBar->setZoomLevel(level);
}

void MainWindow::slotPaste()
{
    static QClipboard* c = QGuiApplication::clipboard();

    const QMimeData *mimeData = c->mimeData();

    if (mimeData->hasFormat(QStringLiteral("x-tutorial4/x-nodes-content"))) {
        const auto a = mimeData->data(QStringLiteral("x-tutorial4/x-nodes-content"));
        currentSession()->addNodesFromData(a, QPoint());
    }
}

void MainWindow::slotBg()
{
    auto d = new color_widgets::ColorDialog(this);

    if (auto cur = m_pSelActionCol->currentNode())
        d->setColor(cur->background().color());

    connect(d, &color_widgets::ColorDialog::colorSelected, this, &MainWindow::applyBg);
    d->show();
}

void MainWindow::slotFg()
{
    auto d = new color_widgets::ColorDialog(this);

    if (auto cur = m_pSelActionCol->currentNode())
        d->setColor(cur->foreground().color());

    connect(d, &color_widgets::ColorDialog::colorSelected, this, &MainWindow::applyFg);
    d->show();
}

void MainWindow::applyBg(const QColor& c)
{
    currentSession()->forEachSelected([c](auto gn, auto) {
        gn->setBackground(c);
    });
    m_pTabs->repaint();
}

void MainWindow::applyFg(const QColor& c)
{
    currentSession()->forEachSelected([c](auto gn, auto) {
        gn->setForeground(c);
    });
    m_pTabs->repaint();
}


bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ContextMenu) {
        slotNodeContextMenu(static_cast<QContextMenuEvent*>(event)->pos());
    }

    return QObject::eventFilter(watched, event);
}

void MainWindow::slotNodeContextMenu(const QPoint& p)
{
    if (!m_pSelActionCol->currentNode()) {
        currentNodeWidget()->selectionModel()->setCurrentIndex(
            currentNodeWidget()->indexAt(p),
            QItemSelectionModel::Clear  |
            QItemSelectionModel::Select
        );

        if (!m_pSelActionCol->currentNode()) {
            auto m = new QMenu(this);

            m->addAction(m_pActionCollection->zoonInAction());
            m->addAction(m_pActionCollection->zoonOutAction());
            m->addAction(m_pActionCollection->zoonFitAction());
            m->addAction(m_pActionCollection->zoonResetAction());
            m->addAction(m_pActionCollection->pasteAction());

            m->exec(QCursor::pos());
            return;
        }
    }

    auto m = new QMenu(this);
    m->addAction(m_pSelActionCol->copy());
    m->addAction(m_pSelActionCol->cut());
    m->addAction(m_pSelActionCol->deleteNode());
    m->addAction(m_pSelActionCol->rename());
    m->addSeparator();
    m->addAction(m_pSelActionCol->bg());
    m->addAction(m_pSelActionCol->fg());
    m->exec(QCursor::pos());
}

#include <mainwindow.moc>
