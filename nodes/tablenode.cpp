#include "tablenode.h"

#include <QtWidgets/QScrollBar>
#include <QtWidgets/QTableView>

#include <QtCore/QDebug>
#include <QtCore/QIdentityProxyModel>
#include <QtCore/QTimer>
#include "widgets/table.h"

#include "common/pagemanager.h"
#include "common/remotemanager.h"
#include "common/abstractsession.h"

class TableNodePrivate : public QObject
{
    Q_OBJECT
public:
    QTableView* m_pTableView;

    Table* m_pTableW {new Table};
    mutable QIdentityProxyModel* m_pRemoteModel {nullptr};
    mutable QString m_Id;

public Q_SLOTS:
    void slotModelChanged(QAbstractItemModel* newModel, QAbstractItemModel* old);
};

TableNode::TableNode(AbstractSession* sess) : ProxyNode(sess), d_ptr(new TableNodePrivate())
{
    d_ptr->m_pTableView  = new QTableView(nullptr);

    QTimer::singleShot(0, [this]() {
        session()->pages()->addPage(this, d_ptr->m_pTableView, title(), uid());
    });

    QObject::connect(this, &ProxyNode::modelChanged, d_ptr, &TableNodePrivate::slotModelChanged);
}

TableNode::~TableNode()
{
    delete d_ptr;
}

QString TableNode::title() const
{
    return QStringLiteral("Table");
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
    Q_UNUSED(old)
    m_pTableView->setModel(newModel);

    /* Scroll to the end */
    QObject::connect(newModel, &QAbstractItemModel::rowsInserted, [this]() {
        if (m_pTableView->verticalScrollBar())
            m_pTableView->verticalScrollBar()->setValue(
                m_pTableView->verticalScrollBar()->maximum()
            );
    });

    if (m_pRemoteModel)
        m_pRemoteModel->setSourceModel(newModel);
}

QString TableNode::remoteWidgetType() const
{
    return id();
}

QString TableNode::remoteModelName() const
{
    if (!d_ptr->m_pRemoteModel) {
        static int count = 1;
        d_ptr->m_Id = id()+QString::number(count++);

        d_ptr->m_pRemoteModel = new QIdentityProxyModel(const_cast<TableNode*>(this));
        d_ptr->m_pRemoteModel->setSourceModel(model());

        RemoteManager::instance()->addModel(d_ptr->m_pRemoteModel, {
            Qt::DisplayRole,
            Qt::EditRole,
        }, d_ptr->m_Id);
    }

    return d_ptr->m_Id;
}

#include <tablenode.moc>
