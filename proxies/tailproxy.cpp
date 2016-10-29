#include "tailproxy.h"

class TailProxyPrivate : public QObject
{
public:
    int m_Maximum {-1};

    TailProxy* q_ptr;

public Q_SLOTS:
    void slotRowsInserted(const QModelIndex &parent, int s, int e);
};

TailProxy::TailProxy(QObject* parent) : QAbstractProxyModel(parent),
d_ptr(new TailProxyPrivate)
{
    d_ptr->q_ptr = this;
}

TailProxy::~TailProxy()
{
    delete d_ptr;
}

int TailProxy::rowCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;

    const int rc = sourceModel()->rowCount(mapToSource(parent));

    if (parent.isValid() || !isLimited())
        return rc;

    return rc > maximum()-1 ? maximum() : rc;
}

int TailProxy::columnCount(const QModelIndex& parent) const
{
    if (!sourceModel())
        return 0;

    return sourceModel()->columnCount(mapToSource(parent));
}

QModelIndex TailProxy::index(int row, int column, const QModelIndex& parent) const
{
    if (!sourceModel())
        return {};

    if ((!parent.isValid()) && isLimited() && row >= maximum())
        return {};

    if (parent.isValid()) {
        const QModelIndex srcParentIdx = mapToSource(parent);
        const QModelIndex srcIdx = sourceModel()->index(row, column, srcParentIdx);

        if (!srcIdx.isValid())
            return {};

        return createIndex(row, column, srcIdx.internalPointer());
    }
    else {
        const QModelIndex srcIdx = sourceModel()->index(row, column);

        if (srcIdx.isValid())
            return createIndex(row, column, srcIdx.internalPointer());
    }

    return {};
}

class HackyDummy : public QAbstractItemModel //FIXME do it using a private QIdentityProxyModel
{
    friend class TailProxy;
};

QModelIndex TailProxy::parent(const QModelIndex& idx) const
{
    if (!sourceModel())
        return {};

    if (!idx.isValid())
        return {};

    const QModelIndex srcParentIdx = mapToSource(idx).parent();

    if (!srcParentIdx.isValid())
        return {};

    // In theory, this function should never be called using an idx whose
    // parent index is within the hidden range. If it does, then this is
    // the consequence, not the source of the bug. It should be safe to ignore
    // this corner case.

    return mapFromSource(idx);
}

QModelIndex TailProxy::mapFromSource(const QModelIndex& sourceIndex) const
{
    if (!sourceModel())
        return {};

    if (!sourceIndex.isValid())
        return {};

    if ((!isLimited()) || sourceIndex.parent().isValid() ||  sourceIndex.row() < maximum())
        return createIndex(
            sourceIndex.row(),
            sourceIndex.column(),
            sourceIndex.internalPointer()
        );

    const int offset = sourceModel()->rowCount() - maximum() - 1;

    return createIndex(
        sourceIndex.row() - offset,
        sourceIndex.column(),
        sourceIndex.internalPointer()
    );
}


bool TailProxy::isCurrentlyLimited() const
{
    return isLimited() && maximum() <= sourceModel()->rowCount();
}


QModelIndex TailProxy::mapToSource(const QModelIndex& proxyIndex) const
{
    if (!sourceModel())
        return {};

    if (!proxyIndex.isValid())
        return {};

    Q_D(const TailProxy);
    Q_ASSERT(proxyIndex.model() == this);

    const bool inRange = !isCurrentlyLimited();

    const int offset = inRange ? 0 : sourceModel()->rowCount() - maximum() - 1;

    // There is 2 choice, being able to use mapFromSource in ::parent() and
    // being hacky here, or do the opposite. This one is a nicer hack


    // Check if the proxyIndex is topLevel, the strange way. This only works
    // because of the way this proxy is implemented. It might seem fragile,
    // but QIdentityProxyModel also does it, lets assume it is right.
    QModelIndex candidate = sourceModel()->index(
        proxyIndex.row() + offset, proxyIndex.column()
    );

    if (candidate.internalPointer() == proxyIndex.internalPointer())
        return candidate;

    // don't ever **EVER** do this at home, **EXPECT** segfaults
    HackyDummy* srcModel = (HackyDummy*) sourceModel();

    return srcModel->createIndex(
        proxyIndex.row(), proxyIndex.column(), proxyIndex.internalPointer()
    );
}

void TailProxy::setSourceModel(QAbstractItemModel *sm)
{
    if (sourceModel())
        disconnect(sourceModel(), &QAbstractItemModel::rowsInserted,
                   d_ptr, &TailProxyPrivate::slotRowsInserted
        );

    QAbstractProxyModel::setSourceModel(sm);

    connect(sourceModel(), &QAbstractItemModel::rowsInserted,
               d_ptr, &TailProxyPrivate::slotRowsInserted
    );
}

bool TailProxy::isLimited() const
{
    return d_ptr->m_Maximum>-1;
}

int TailProxy::maximum() const
{
    return d_ptr->m_Maximum;
}

void TailProxy::setMaximum(int max)
{
    if (max < 0)
        return;

    d_ptr->m_Maximum = max;

    if (sourceModel() && sourceModel()->rowCount() > max)
        Q_EMIT layoutChanged();
}

void TailProxy::setLimited(bool limit)
{
    const bool differ = isLimited() != limit;
    d_ptr->m_Maximum = limit ? d_ptr->m_Maximum : -1;

    if (differ)
        Q_EMIT layoutChanged();
}

void TailProxyPrivate::slotRowsInserted(const QModelIndex &parent, int s, int e)
{
    if (q_ptr->isCurrentlyLimited())
        Q_EMIT q_ptr->dataChanged(
            q_ptr->index(0,0),
            q_ptr->index(q_ptr->maximum()-1, q_ptr->columnCount())
        );
    else {
        q_ptr->beginInsertRows(parent, s, e);
        q_ptr->endInsertRows();
    }
}
