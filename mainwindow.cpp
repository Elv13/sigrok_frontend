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

#include "nodes/devicenode.h"
#include "nodes/chartnode.h"
#include "nodes/tablenode.h"
#include "nodes/meternode.h"
#include "nodes/columnnode.h"
#include "nodes/colornode.h"

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

    auto deviceAdapter = new NodeAdapter<DeviceNode>(
        devm, _scene, 1, (int) DeviceModel::Role::DEVICE_NODE
    );

    auto n = new GraphicsNode();
    n->setTitle("Devices");
    n->setCentralWidget(new DeviceList());
    for (int j = 0; j < devm->rowCount(); j++) {
        n->setPos(j * 25, j * 25);
        n->add_source(devm->data(devm->index(j,0), Qt::DisplayRole).toString());
    }
    _scene->addItem(n);

    //TODO do not hardcode
//     auto chartType = new ChartType();


    auto pna = new ProxyNodeFactoryAdapter(_scene);

    setCentralWidget(w);

    pna->registerType<ChartNode> ("Chart"          , "Widgets", QIcon::fromTheme( "document-edit"        ));
    pna->registerType<TableNode> ("Table"          , "Widgets", QIcon::fromTheme( "configure-shortcuts"  ));
    pna->registerType<MeterNode> ("Meter"          , "Widgets", QIcon::fromTheme( "bookmark-new"         ));
    pna->registerType<ColumnNode>("Range filter"   , "Filters", QIcon::fromTheme( "view-filter"          ));
    pna->registerType<ColorNode> ("Range Colorizer", "Metadata", QIcon::fromTheme( "colors-chromablue"   ));
    pna->registerType<ColumnNode>("Column filter"  , "Filters", QIcon::fromTheme( "view-filter"          ));

    //DUMMY
    pna->registerType<ColorNode> ("Memento"         , "Sources", QIcon::fromTheme( "view-calendar-timeline"          ));
    pna->registerType<ColorNode> ("Live aquisition" , "Sources", QIcon::fromTheme( "view-calendar-timeline"          ));
    pna->registerType<ColorNode> ("File"            , "Sources", QIcon::fromTheme( "document-open"          ));
    pna->registerType<ColorNode> ("Statistics"      , "Metadata", QIcon::fromTheme( "format-number-percent"        ));
    pna->registerType<ColorNode> ("Chronometer"     , "Metadata", QIcon::fromTheme( "chronometer"        ));
    pna->registerType<ColorNode> ("Copy"            , "Multipler",QIcon::fromTheme( "edit-copy"          ));
    pna->registerType<ColorNode> ("CSV"             , "Exporter", QIcon::fromTheme( "document-save"      ));
    pna->registerType<ColorNode> ("XLSX"            , "Exporter", QIcon::fromTheme( "document-share"     ));
    pna->registerType<ColorNode> ("ODS"             , "Exporter", QIcon::fromTheme( "document-save-all"  ));
    pna->registerType<ColorNode> ("PCAP (WireShark)", "Exporter", QIcon::fromTheme( "document-save-as"   ));

    pna->registerType<ColorNode> ("Rate watchdog"   , "Sinks"     , QIcon::fromTheme( "mail-forward"     ));
    pna->registerType<ColorNode> ("Tail filter"     , "Filters"   , QIcon::fromTheme( "kt-add-filters"   ));
    pna->registerType<ColorNode> ("Head filter"     , "Filters"   , QIcon::fromTheme( "kt-remove-filters"));

    //Create the node creator dock
    auto dock = new QDockWidget    ( instance );
    auto tab  = new CategorizedTree( dock     );
    tab->setModel(pna);
    auto del = new CategorizedDelegate(tab);
    del->setChildDelegate(new AutoCompletionDelegate());
    tab->setItemDelegate(del);
    tab->setIndentation(5);
    tab->expandAll();
    QObject::connect(tab, &QTreeView::doubleClicked, [pna](const QModelIndex& idx) {
        pna->addToScene(idx);
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
    if (!outputFileName.isNull())
    {
        QSaveFile file(outputFileName);
        file.open(QIODevice::WriteOnly);

        QByteArray outputByteArray;
        outputByteArray.append(textArea->toPlainText().toUtf8());
        file.write(outputByteArray);
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
    if (!fileName.isEmpty())
    {
        saveFileAs(fileName);
    }
    else
    {
        saveFileAs();
    }
}


void MainWindow::openFile()
{
    QUrl fileNameFromDialog = QFileDialog::getOpenFileUrl(this, i18n("Open File"));

    if (!fileNameFromDialog.isEmpty())
    {
        KIO::Job* job = KIO::storedGet(fileNameFromDialog);
        fileName = fileNameFromDialog.toLocalFile();

        connect(job, SIGNAL(result(KJob*)), this, SLOT(downloadFinished(KJob*)));

        job->exec();
    }
}

void MainWindow::downloadFinished(KJob* job)
{
    if (job->error())
    {
        KMessageBox::error(this, job->errorString());
        fileName.clear();
        return;
    }

    KIO::StoredTransferJob* storedJob = (KIO::StoredTransferJob*)job;
    textArea->setPlainText(QTextStream(storedJob->data(), QIODevice::ReadOnly).readAll());
}
