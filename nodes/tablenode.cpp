#include "tablenode.h"

#include "../widgets/aquisition.h"

#include <QtWidgets/QScrollBar>
#include <QRemoteObjectHost>
#include <QtWidgets/QTableView>

#include <QtCore/QDebug>

#include "../widgets/table.h"

#include "../mainwindow.h"

class TableNodePrivate : public QObject
{
public:
    QTableView* m_pTableView;

    Table* m_pTableW {new Table};

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

TableNode::TableNode(QObject* parent) : ProxyNode(parent), d_ptr(new TableNodePrivate())
{
    d_ptr->m_pTableView  = new QTableView(nullptr);
    MainWindow::addDock(d_ptr->m_pTableView, "Table");

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &TableNodePrivate::slotModelChanged);
}

TableNode::~TableNode()
{
    
}

QString TableNode::title() const
{
    return "Table";
}

QWidget* TableNode::widget() const
{
    return d_ptr->m_pTableW;
}

void TableNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pTableView->setModel(newModel);

    qDebug() << "\n\n\nDSFSFSD";
    auto node = new QRemoteObjectRegistryHost(QUrl(QStringLiteral("tcp://10.10.10.136:2223")));

    QVector<int> roles;
    roles << Qt::DisplayRole << Qt::BackgroundRole;

    auto node2 = new QRemoteObjectHost (QUrl(QStringLiteral("tcp://10.10.10.136:2224")), QUrl(QStringLiteral("tcp://10.10.10.136:2223")));
    node2->enableRemoting(newModel, QStringLiteral("RemoteModel"), roles);

    /* Scroll to the end */
    QObject::connect(newModel, &QAbstractItemModel::rowsInserted, [this]() {
        if (m_pTableView->verticalScrollBar())
            m_pTableView->verticalScrollBar()->setValue(
                m_pTableView->verticalScrollBar()->maximum()
            );
    });
}
