#include "rangeproxy.h"

#include <QtCore/QDebug>

#include <QtWidgets/QWidget>
#include <QtWidgets/QAbstractItemView>

class RangeProxyPrivate : public QObject
{
public:
    QVector<bool> m_lShow;
    int m_ExtraColumnCount {0};
    QAbstractItemView* m_pWidget {nullptr};
    QVector< std::function<QWidget*(int)> > m_lWidgetFactories;

    RangeProxy* q_ptr;

public Q_SLOTS:
    void slotColumnInserted();
};

RangeProxy::RangeProxy(QObject* parent) : QIdentityProxyModel(parent),
    d_ptr(new RangeProxyPrivate())
{
    d_ptr->q_ptr = this;

    QObject::connect(this, &QAbstractItemModel::rowsInserted,
        d_ptr, &RangeProxyPrivate::slotColumnInserted);
    QObject::connect(this, &QAbstractItemModel::columnsInserted,
        d_ptr, &RangeProxyPrivate::slotColumnInserted);
    QObject::connect(this, &QAbstractItemModel::layoutChanged,
        d_ptr, &RangeProxyPrivate::slotColumnInserted);
}


int RangeProxy::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 3;
}

QVariant RangeProxy::headerData(int sec, Qt::Orientation ori, int role) const
{
    return QIdentityProxyModel::headerData(sec, ori, role);
}

QModelIndex RangeProxy::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() && column > 0 && column <= d_ptr->m_ExtraColumnCount) {
        return createIndex(row, column, nullptr);
    }

    return QIdentityProxyModel::index(row, column, parent);
}

Qt::ItemFlags RangeProxy::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags ret = QIdentityProxyModel::flags(idx);

    return ret | Qt::ItemIsUserCheckable;
}

QVariant RangeProxy::data(const QModelIndex& idx, int role) const
{
    if (idx.column() > 0) return {};

    return QIdentityProxyModel::data(idx, role);
}

bool RangeProxy::setData(const QModelIndex &i, const QVariant &v, int r)
{
    return false;
}

int RangeProxy::extraColumnCount() const
{
    return d_ptr->m_ExtraColumnCount;
}

void RangeProxy::setExtraColumnCount(int value)
{
    d_ptr->m_ExtraColumnCount = value;
    d_ptr->slotColumnInserted();
}

void RangeProxy::setColumnWidgetFactory(int col, std::function<QWidget*(int)> w)
{
    const int cc = columnCount();
    if (col > cc-1) return;

    d_ptr->m_lWidgetFactories.resize(cc);

    d_ptr->m_lWidgetFactories[col] = w;
    d_ptr->slotColumnInserted();
}

void RangeProxy::setWidget(QAbstractItemView* w)
{
    d_ptr->m_pWidget = w;
    d_ptr->slotColumnInserted();
}

void RangeProxyPrivate::slotColumnInserted()
{
    if (!m_pWidget) return;

    // Create all missing widgets
    for (int i=0; i < q_ptr->rowCount(); i++) {
        for (int j = 0; j < m_ExtraColumnCount; j++) {
            const QModelIndex idx = m_pWidget->model()->index(i,j+1);
            auto w = m_pWidget->indexWidget(idx);
            if ((!w) && m_lWidgetFactories.size() > j && m_lWidgetFactories[j+1]) {
                w = m_lWidgetFactories[j+1](i);
                m_pWidget->setIndexWidget(idx, w);
            }
        }
    }
}
