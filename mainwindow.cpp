#include <QApplication>
#include <QAction>
#include <QSaveFile>
#include <QFileDialog>
#include <QTextStream>
#include <QByteArray>

#include <KTextEdit>
#include <KLocalizedString>
#include <KActionCollection>
#include <KStandardAction>
#include <KMessageBox>
#include <KIO/Job>

#include "devicemodel.h"
#include "aquisitionmodel.h"
#include "nodeadapter.h"
#include "proxynodefactory.h"
#include "models/remotewidgets.h"

#include "nodes/devicenode.h"
#include "nodes/chartnode.h"
#include "nodes/tablenode.h"
#include "nodes/meternode.h"
#include "nodes/columnnode.h"
#include "nodes/colornode.h"
#include "nodes/mementonode.h"
#include "nodes/multiplexernode.h"

#include "widgets/devicelist.h"
#include "widgets/charttype.h"
#include "widgets/categorizedtree.h"
#include "widgets/statusbar.h"

#include "delegates/categorizeddelegate.h"
#include "delegates/autocompletiondelegate.h"

#include <QtCore/QDebug>

#include <QtWidgets/QDockWidget>
#include <QtWidgets/QTreeView>

#include "mainwindow.h"

#include "qt5-node-editor/src/graphicsnode.hpp"
#include "qt5-node-editor/src/graphicsnodescene.hpp"
#include "qt5-node-editor/src/modelnode.hpp"

#include <libsigrokcxx/libsigrokcxx.hpp>

static MainWindow* instance; //FIXME

QDockWidget* MainWindow::addDock(QWidget* w, const QString& title)
{
    auto dock = new QDockWidget(instance);
    dock->setWidget(w);
    instance->addDockWidget(Qt::TopDockWidgetArea, dock);

    return dock;
}

MainWindow::MainWindow(QWidget *parent) : KXmlGuiWindow(parent), fileName(QString())
{
    instance = this;

    m_pStatusBar = new StatusBar2(this);
    setStatusBar(m_pStatusBar);

    auto w = new QWidget(this);
    setupUi(w);

    auto devm = DeviceModel::instance();

    auto _scene = new GraphicsNodeScene(this);
    _scene->setSceneRect(-32000, -32000, 64000, 64000);
    m_pNode->setScene(_scene);

    auto devnode = new Modelnode(devm);
    devnode->setObjectRole((int) DeviceModel::Role::DEVICE);
    devnode->setIdRole((int) DeviceModel::Role::DEVICE);
    _scene->addItem(devnode);

    auto remotenode = new Modelnode(new RemoteWidgets(this));
    remotenode->setConnectedObjectRole(999);
    remotenode->setConnectedPropertyRole(998);
    _scene->addItem(remotenode);

    m_pSession = new ProxyNodeFactoryAdapter(_scene);

    setCentralWidget(w);

    m_pSession->registerType<ChartNode>  ("Chart"          , "Widgets"   , "chart_node", QIcon::fromTheme( "document-edit"        ));
    m_pSession->registerType<TableNode>  ("Table"          , "Widgets"   , "table_node", QIcon::fromTheme( "configure-shortcuts"  ));
    m_pSession->registerType<MeterNode>  ("Meter"          , "Widgets"   , "meter_node", QIcon::fromTheme( "bookmark-new"         ));
    m_pSession->registerType<ColumnNode> ("Range filter"   , "Filters"   , "range_node", QIcon::fromTheme( "view-filter"          ));
    m_pSession->registerType<ColorNode>  ("Range Colorizer", "Metadata"  , "color_node", QIcon::fromTheme( "colors-chromablue"   ));
    m_pSession->registerType<ColumnNode> ("Column filter"  , "Filters"   , "column_node", QIcon::fromTheme( "view-filter"          ));
    m_pSession->registerType<DeviceNode> ("Live aquisition" , "Sources"  , "device_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<MementoNode>("Memento"         , "Sources"  , "memento_node", QIcon::fromTheme( "view-calendar-timeline"          ));
    m_pSession->registerType<MultiplexerNode>("Multiplexer" , "Tools"    , "multiplexer_node", QIcon::fromTheme( "edit-copy"          ));

    //DUMMY
    m_pSession->registerType<ColorNode> ("File"            , "Sources"  , " ", QIcon::fromTheme( "document-open"          ));
    m_pSession->registerType<ColorNode> ("External device" , "Sources"  , " ", QIcon::fromTheme( "document-share"          )); //allow async memento transfer from other instances
    m_pSession->registerType<ColorNode> ("Statistics"      , "Metadata" , " ", QIcon::fromTheme( "format-number-percent"        ));
    m_pSession->registerType<ColorNode> ("Chronometer"     , "Metadata" , " ", QIcon::fromTheme( "chronometer"        ));
    m_pSession->registerType<ColorNode> ("Timer"           , "Tools"    , " ", QIcon::fromTheme( "edit-copy"          ));
    m_pSession->registerType<ColorNode> ("CSV"             , "Exporter" , " ", QIcon::fromTheme( "document-save"      ));
    m_pSession->registerType<ColorNode> ("XLSX"            , "Exporter" , " ", QIcon::fromTheme( "document-share"     ));
    m_pSession->registerType<ColorNode> ("ODS"             , "Exporter" , " ", QIcon::fromTheme( "document-save-all"  ));
    m_pSession->registerType<ColorNode> ("PCAP (WireShark)", "Exporter" , " ", QIcon::fromTheme( "document-save-as"   ));

    m_pSession->registerType<ColorNode> ("Rate watchdog"   , "Sinks"    , "" , QIcon::fromTheme( "mail-forward"     ));
    m_pSession->registerType<ColorNode> ("Tail filter"     , "Filters"  , "" , QIcon::fromTheme( "kt-add-filters"   ));
    m_pSession->registerType<ColorNode> ("Head filter"     , "Filters"  , "" , QIcon::fromTheme( "kt-remove-filters"));
    m_pSession->registerType<ColorNode> ("Unit filter"     , "Filters"  , "" , QIcon::fromTheme( "kt-remove-filters"));

    //Create the node creator dock
    auto dock = new QDockWidget    ( instance );
    auto tab  = new CategorizedTree( dock     );
    tab->setModel(m_pSession);
    auto del = new CategorizedDelegate(tab);
    del->setChildDelegate(new AutoCompletionDelegate());
    tab->setItemDelegate(del);
    tab->setIndentation(5);
    tab->expandAll();
    QObject::connect(tab, &QTreeView::doubleClicked, [this](const QModelIndex& idx) {
        m_pSession->addToScene(idx);
    });

    dock->setWidget(tab);
    instance->addDockWidget(Qt::LeftDockWidgetArea, dock);

    setupActions();

}

void MainWindow::setupActions()
{
    QAction* clearAction = new QAction(this);
    clearAction->setText(i18n("&Clear"));
    clearAction->setIcon(QIcon::fromTheme("document-new"));
    actionCollection()->setDefaultShortcut(clearAction, Qt::CTRL + Qt::Key_W);
    actionCollection()->addAction("clear", clearAction);
//     connect(clearAction, SIGNAL(triggered(bool)), textArea, SLOT(clear()));

    KStandardAction::quit(qApp, SLOT(quit()), actionCollection());

    KStandardAction::open(this, SLOT(openFile()), actionCollection());

    KStandardAction::save(this, SLOT(saveFile()), actionCollection());

    KStandardAction::saveAs(this, SLOT(saveFileAs()), actionCollection());

    KStandardAction::openNew(this, SLOT(newFile()), actionCollection());

    setupGUI(Default, "tutorial4ui.rc");
}

void MainWindow::newFile()
{
    fileName.clear();
    textArea->clear();
}

void MainWindow::saveFileAs(const QString &outputFileName)
{
    if (!outputFileName.isNull()) {
        QSaveFile file(outputFileName);
        file.open(QIODevice::WriteOnly);

//         QByteArray outputByteArray;
//         outputByteArray.append(textArea->toPlainText().toUtf8());
//         file.write(outputByteArray);
        m_pSession->serialize(&file);
        file.commit();

        fileName = outputFileName;
    }
}

void MainWindow::saveFileAs()
{
    saveFileAs(QFileDialog::getSaveFileName(this, i18n("Save File As")));
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


void MainWindow::openFile()
{
    QUrl fileNameFromDialog = QFileDialog::getOpenFileUrl(this, i18n("Open File"));

    if (!fileNameFromDialog.isEmpty()) {
        KIO::Job* job = KIO::storedGet(fileNameFromDialog);
        fileName = fileNameFromDialog.toLocalFile();

        connect(job, SIGNAL(result(KJob*)), this, SLOT(downloadFinished(KJob*)));

        job->exec();
    }
}

void MainWindow::downloadFinished(KJob* job)
{
    if (job->error()) {
        KMessageBox::error(this, job->errorString());
        fileName.clear();
        return;
    }

    KIO::StoredTransferJob* storedJob = (KIO::StoredTransferJob*)job;
//     textArea->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());

    m_pSession->load(storedJob->data());
}
