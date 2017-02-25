#include "vectorizingproxy.h"

#include <QtCore/QDebug>

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class VectorizingProxyPrivate : public QObject
{
    Q_OBJECT
public:
    QVector<std::shared_ptr< QVector<double> > > m_lData;

    // In my use case, I need both x and y...
    QVector<double> m_lIds;

    VectorizingProxy* q_ptr;
public Q_SLOTS:
    void slotLayoutChanged();
    void slotRowsInserted(const QModelIndex &parent, int first, int last);
    void slotRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last);
};

VectorizingProxy::VectorizingProxy(QObject* parent) : QIdentityProxyModel(parent),
    d_ptr(new VectorizingProxyPrivate)
{
    d_ptr->q_ptr = this;
    connect(this, &QAbstractItemModel::rowsInserted,
            d_ptr, &VectorizingProxyPrivate::slotRowsInserted);
    connect(this, &QAbstractItemModel::rowsAboutToBeRemoved,
            d_ptr, &VectorizingProxyPrivate::slotRowsAboutToBeRemoved);
    connect(this, &QAbstractItemModel::layoutChanged,
            d_ptr, &VectorizingProxyPrivate::slotLayoutChanged);
    connect(this, &QAbstractItemModel::modelReset,
            d_ptr, &VectorizingProxyPrivate::slotLayoutChanged);
}

VectorizingProxy::~VectorizingProxy()
{
    delete d_ptr;
}

void VectorizingProxy::setColumns(const QList<int>& columns)
{
    int max = 0;

    for (const auto c : qAsConst(columns))
        max = std::max(max, c);

    if (sourceModel() && sourceModel()->columnCount() <= max)
        qWarning() << "Trying to cache a column that doesn't exist yet";

    d_ptr->m_lData.clear();
    d_ptr->m_lData.resize(max+1);

    for (const auto c : qAsConst(columns))
        d_ptr->m_lData[c] = std::make_shared<QVector<double>>(
            sourceModel() ? sourceModel()->rowCount() : 0
        );
}

const QVector<double> VectorizingProxy::cowVectorData(int column) const
{
    return *sharedVectorData(column).get();
}

std::shared_ptr< QVector<double> > VectorizingProxy::
sharedVectorData(int column) const
{
    if (column >= d_ptr->m_lData.size())
        return nullptr;

    return d_ptr->m_lData[column];
}

void VectorizingProxy::setSourceModel(QAbstractItemModel* model)
{
    QIdentityProxyModel::setSourceModel(model);
    d_ptr->slotLayoutChanged();
}

void VectorizingProxyPrivate::slotLayoutChanged()
{
    if (!q_ptr->sourceModel())
        return;

    const int rc = q_ptr->sourceModel()->rowCount();

    // Resize all vectors
    for (int j =0; j < m_lData.size(); j++) {
        if (auto dtptr = m_lData[j])
            dtptr.get()->resize(rc);
    }

    m_lIds.resize(rc);

    // Using a copy instead of calling slotRowsInserted so OpenMP can
    // be enabled. It's not useful to use parallelism for inserted rows as the
    // count is almost always small.
    #pragma omp parallel for
    for (int i = 0; i < rc; i++) {
        for (int j =0; j < m_lData.size(); j++) {
            if (auto dtptr = m_lData[j]) {
                QVector<double>& vector = *dtptr.get();

                vector[i] = q_ptr->sourceModel()->index(i,j).data().toDouble();
            }
        }
        m_lIds[i] = i;
    }
}

void VectorizingProxyPrivate::
slotRowsInserted(const QModelIndex &parent, int first, int last)
{
    if (parent.isValid())
        return;

    const int newSize = m_lIds.size() + (last - first) + 1;
    // Resize all vectors
    for (int j =0; j < m_lData.size(); j++) {
        if (auto dtptr = m_lData[j])
            dtptr.get()->resize(newSize);
    }
    m_lIds.resize(newSize);

    // Add the values
    for (int i = first; i <= last; i++) {
        for (int j =0; j < m_lData.size(); j++) {
            if (auto dtptr = m_lData[j]) {
                QVector<double>& vector = *dtptr.get();
                const auto idx = q_ptr->sourceModel()->index(i,j);
                vector[i] = idx.data().toDouble();
                qDebug() << i << j << vector[i] << idx << idx.data();
            }
        }
        m_lIds[i] = i;
    }
}

void VectorizingProxyPrivate::
slotRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    Q_UNUSED(parent);
    Q_UNUSED(first);
    Q_UNUSED(last);
    qDebug() << "Your graph output is now worthless, sorry";
    //TODO
}

QVector<double> VectorizingProxy::rowsVector() const
{
    return d_ptr->m_lIds;
}

#include <vectorizingproxy.moc>
