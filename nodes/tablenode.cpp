#include "tablenode.h"

#include "widgets/aquisition.h"

#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableView>

#include <QtCore/QDebug>

#include "widgets/table.h"

#include "common/pagemanager.h"

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
    PageManager::instance()->addPage(d_ptr->m_pTableView, "Table");

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &TableNodePrivate::slotModelChanged);
}

TableNode::~TableNode()
{
    
}

QString TableNode::title() const
{
    return "Table";
}

QString TableNode::id() const
{
    return QStringLiteral("table_node");
}

void TableNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);

    
}

QWidget* TableNode::widget() const
{
    return d_ptr->m_pTableW;
}

void TableNodePrivate::slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old)
{
    m_pTableView->setModel(newModel);

    /* Scroll to the end */
    QObject::connect(newModel, &QAbstractItemModel::rowsInserted, [this]() {
        if (m_pTableView->verticalScrollBar())
            m_pTableView->verticalScrollBar()->setValue(
                m_pTableView->verticalScrollBar()->maximum()
            );
    });
}
