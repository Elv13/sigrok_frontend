#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QPushButton>
#include <QRemoteObjectNode>
#include <QAbstractItemModelReplica>

#include "common/pagemanager.h"

#include "meterdata_replica.h"

void MainWindow::addDock(QWidget* w, const QString& title)
{
    static QHash<QString, int> uniqueNames;

    ui->m_pWidgetsL->addWidget(w);
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)/*,
    ui(new Ui::MainWindow)*/
{
    auto mw = new QWidget(this);
    setCentralWidget(mw);

    ui->setupUi(mw);

    connect(PageManager::instance(), &PageManager::pageAdded, this, &MainWindow::addDock);

    auto registry = new QRemoteObjectNode(QUrl(QStringLiteral("tcp://10.10.10.136:2223")));
    registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2224")));
    registry->connectToNode(QUrl(QStringLiteral("tcp://10.10.10.136:2225")));

    MeterDataReplica* md = registry->acquire<MeterDataReplica>();
//     rw = registry->acquire<RemoteWidgetsReplica>();

    auto model = registry->acquireModel(QStringLiteral("RemoteModel"));
    //ui->treeView->setModel(model);

    qDebug() << "BEFORE" << md->mainValue() << md->isReplicaValid() << md->isInitialized();
    connect(md, SIGNAL(mainValueChanged(float)), this, SLOT(setMainLabel(float)));
    qDebug() << "AFTER";

//     connect(rw, SIGNAL(namesChanged(QStringList)), this, SLOT(reloadremotewidgets()));
    setMainLabel(md->mainValue());

    reloadremotewidgets();
}

MainWindow::~MainWindow()
{
    //delete ui;
}

void MainWindow::setMainLabel(float value)
{
//     ui->lcdNumber->setValue(value);
}

void MainWindow::reloadremotewidgets()
{
//     qDebug() << "REMOTE WIDGETS CHANGED";
//     const int current = ui->verticalLayout->count();
//     if (rw->names().size() > current) {
//         for (int i = current; i < rw->names().size(); i++) {
//            auto pb = new QPushButton(this);
//            pb->setText(rw->names()[i]);
//            pb->setProperty("rid", i);
//            ui->verticalLayout->addWidget(pb);
//            connect(pb, &QPushButton::clicked, this, &MainWindow::buttonPressed);
//         }
//     }
}

void MainWindow::buttonPressed()
{
//     const int id = QObject::sender()->property("rid").toInt();
//     rw->activated(id, true);
}
