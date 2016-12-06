#include "rowsdeletionproxy.h"

#include <QtCore/QItemSelectionModel>
#include <QtCore/QDebug>
#include <QtCore/QTimer>

class RowsDeletionProxyPrivate
{
public:
    QVariant m_Icon;
    bool m_Mutex {false};
};

RowsDeletionProxy::RowsDeletionProxy(QObject* parent) :
    QIdentityProxyModel(parent), d_ptr(new RowsDeletionProxyPrivate)
{

}

RowsDeletionProxy::~RowsDeletionProxy()
{
    delete d_ptr;
}

QVariant RowsDeletionProxy::data(const QModelIndex& idx, int role) const
{
    if (idx.column() +1 == columnCount(idx.parent()))
        return (role == Qt::DecorationRole) ? d_ptr->m_Icon : QVariant();

    return QIdentityProxyModel::data(idx, role);
}

int RowsDeletionProxy::columnCount(const QModelIndex& parent) const
{
    return QIdentityProxyModel::columnCount(parent) + 1;
}

QModelIndex RowsDeletionProxy::index(int row, int column, const QModelIndex& parent) const
{
    if(column + 1 == columnCount(parent))
        return createIndex(row, column, Q_NULLPTR);

    return QIdentityProxyModel::index(row, column, parent);
}

void RowsDeletionProxy::setSelectionModel(QItemSelectionModel* sm)
{
    Q_ASSERT(sm->model() == this);
    connect(sm, &QItemSelectionModel::currentChanged, [this, sm](const QModelIndex& idx) {
        if (d_ptr->m_Mutex == false && idx.isValid() && idx.column()+1 == columnCount(idx.parent())) {
            d_ptr->m_Mutex = true;

            // The view is also connected to this signal, this create a race
            // condition and remove 2 rows. Delaying the removeRow() solve this
            QPersistentModelIndex pIdx(idx);
            QTimer::singleShot(0,[pIdx, sm, this]() {
                if (!pIdx.isValid())
                    return;

                sm->setCurrentIndex({}, QItemSelectionModel::Clear);
                Q_ASSERT(!sm->currentIndex().isValid());

                removeRow(pIdx.row(), pIdx.parent());
                sm->setCurrentIndex({}, QItemSelectionModel::Clear);
                d_ptr->m_Mutex = false;
            });
        }
    });
}

void RowsDeletionProxy::setIcon(const QVariant& icon)
{
    d_ptr->m_Icon = icon;
}
