#include "qmultimodeltree.h"

#include <QtCore/QDebug>
#include <QtCore/QDataStream>
#include <QtCore/QMimeData>
#include <QtCore/QMimeType>

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

struct InternalItem
{
    enum class Mode {
        ROOT,
        PROXY
    };

    int                    m_Index;
    Mode                   m_Mode;
    QAbstractItemModel*    m_pModel;
    InternalItem*          m_pParent;
    QVector<InternalItem*> m_lChildren;
    QString                m_Title;
    QVariant               m_UId, m_Bg, m_Fg, m_Deco;
};

class QMultiModelTreePrivate : public QObject
{
public:
    explicit QMultiModelTreePrivate(QObject* p) : QObject(p) {}

    QVector<InternalItem*> m_lRows;
    QHash<const QAbstractItemModel*, InternalItem*> m_hModels;

    bool m_HasIdRole {false};
    int m_IdRole {Qt::DisplayRole};

    QMultiModelTree* q_ptr;

public Q_SLOTS:
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
    void slotDataChanged(const QModelIndex& tl, const QModelIndex& br);
    void slotAddRows(const QModelIndex& parent, int first, int last, QAbstractItemModel* src);
};

QMultiModelTree::QMultiModelTree(QObject* parent) : QAbstractItemModel(parent),
    d_ptr(new QMultiModelTreePrivate(this))
{
    d_ptr->q_ptr = this;
}

QMultiModelTree::~QMultiModelTree()
{
    d_ptr->m_hModels.clear();
    while(!d_ptr->m_lRows.isEmpty()) {
        while(!d_ptr->m_lRows.first()->m_lChildren.isEmpty())
            delete d_ptr->m_lRows.first()->m_lChildren.first();
        delete d_ptr->m_lRows.first();
    }

    delete d_ptr;
}

QAbstractItemModel* QMultiModelTree::getModel(const QModelIndex& _idx) const
{
    auto idx = _idx;

    if (idx.parent().isValid())
        idx = idx.parent();

    if ((!idx.isValid()) || idx.model() != this || idx.parent().isValid())
        return Q_NULLPTR;

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    Q_ASSERT(!i->m_pParent);
    Q_ASSERT(i->m_pModel);

    return i->m_pModel;
}

QVariant QMultiModelTree::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    if (i->m_Mode == InternalItem::Mode::PROXY)
        return i->m_pModel->data(mapToSource(idx), role);

    if (d_ptr->m_HasIdRole && d_ptr->m_IdRole == role)
        return i->m_UId;

    switch (role) {
        case Qt::BackgroundRole:
            return i->m_Bg;
        case Qt::ForegroundRole:
            return i->m_Fg;
        case Qt::DecorationRole:
            return i->m_Deco;
        case Qt::DisplayRole:
            return (!i->m_Title.isEmpty()) ?
                i->m_Title : i->m_pModel->objectName();
    };

    return {};
}

bool QMultiModelTree::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid())
        return {};

    auto i = static_cast<InternalItem*>(index.internalPointer());

    switch(i->m_Mode) {
        case InternalItem::Mode::PROXY:
            return i->m_pModel->setData(mapToSource(index), value, role);
        case InternalItem::Mode::ROOT:
            if (d_ptr->m_HasIdRole && d_ptr->m_IdRole == role) {
                i->m_UId = value;
                Q_EMIT dataChanged(index, index);
                return true;
            }
            switch(role) {
                case Qt::BackgroundRole:
                    i->m_Bg = value;
                    Q_EMIT dataChanged(index, index);
                    return true;
                case Qt::ForegroundRole:
                    i->m_Fg = value;
                    Q_EMIT dataChanged(index, index);
                    return true;
                case Qt::DecorationRole:
                    i->m_Deco = value;
                    Q_EMIT dataChanged(index, index);
                    return true;
                case Qt::DisplayRole:
                case Qt::EditRole: {
                    const auto oldT = i->m_Title;
                    const auto newT = value.toString();
                    i->m_Title = newT;
                    Q_EMIT dataChanged(index, index);
                    if (newT != oldT) {
                        Q_EMIT modelRenamed(i->m_pModel, newT, oldT);
                        Q_EMIT modelRenamed(index, newT, oldT);
                    }
                    return true;
                }
                break;
            }
            break;
    };

    return false;
}

int QMultiModelTree::rowCount(const QModelIndex& parent) const
{
    if (!parent.isValid())
        return d_ptr->m_lRows.size();

    const auto i = static_cast<InternalItem*>(parent.internalPointer());

    if (i->m_Mode == InternalItem::Mode::PROXY)
        return 0;

    return i->m_pModel->rowCount();
}

int QMultiModelTree::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return 1;
}

QModelIndex QMultiModelTree::index(int row, int column, const QModelIndex& parent) const
{
    if (
        (!parent.isValid())
        && row >= 0
        && row < d_ptr->m_lRows.size()
        && column == 0
    )
        return createIndex(row, column, d_ptr->m_lRows[row]);

    if ((!parent.isValid()) || parent.model() != this)
        return {};


    const auto i = static_cast<InternalItem*>(parent.internalPointer());

    return mapFromSource(i->m_pModel->index(row, column));
}

Qt::ItemFlags QMultiModelTree::flags(const QModelIndex &idx) const
{
    const auto pidx = mapToSource(idx);
    return pidx.isValid() ?
        pidx.flags() : Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex QMultiModelTree::parent(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return {};

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    if (i->m_Mode == InternalItem::Mode::ROOT)
        return {};

    return createIndex(i->m_pParent->m_Index, 0, i->m_pParent);
}

QModelIndex QMultiModelTree::mapFromSource(const QModelIndex& sourceIndex) const
{
    if ((!sourceIndex.isValid()) || sourceIndex.parent().isValid() || sourceIndex.column())
        return {};

    const auto i = d_ptr->m_hModels[sourceIndex.model()];

    if ((!i) || sourceIndex.row() >= i->m_lChildren.size())
        return {};

    return createIndex(sourceIndex.row(), 0, i->m_lChildren[sourceIndex.row()]);
}

QModelIndex QMultiModelTree::mapToSource(const QModelIndex& proxyIndex) const
{
    if ((!proxyIndex.isValid()) || proxyIndex.model() != this)
        return {};

    if (!proxyIndex.parent().isValid())
        return {};

    const auto i = static_cast<InternalItem*>(proxyIndex.internalPointer());

    return i->m_pModel->index(proxyIndex.row(), proxyIndex.column());
}

bool QMultiModelTree::removeRows(int row, int count, const QModelIndex &parent)
{
    if (row < 0 || count < 1)
        return false;

    auto idx = parent;

    while (idx.parent().isValid())
        idx = idx.parent();

    if (idx.isValid() && !idx.parent().isValid()) {
        const auto i = static_cast<InternalItem*>(idx.internalPointer());
        if (i->m_pModel->removeRows(row, count, mapToSource(parent))) {
            beginRemoveRows(parent, row, row + count - 1);
            endRemoveRows();
            return true;
        }
        //FIXME update the m_Index
    }
    else if (!parent.isValid() && row + count <= d_ptr->m_lRows.size()) {

        beginRemoveRows(parent, row, row + count - 1);
            for(int i = row; i < row+count; i++) {
                auto item = d_ptr->m_lRows[i];
                d_ptr->m_hModels.remove(item->m_pModel);

                delete item;
            }

            d_ptr->m_lRows.remove(row, count);

            for (int i = row+count-1; i < rowCount(); i++)
                d_ptr->m_lRows[i]->m_Index = i;

        endRemoveRows();

        return true;
    }

    return false;
}

void QMultiModelTreePrivate::slotAddRows(const QModelIndex& parent, int first, int last, QAbstractItemModel* src)
{
    if (parent.isValid()) return;

    auto p = m_hModels[src];
    Q_ASSERT(p);

    const auto localParent = q_ptr->index(p->m_Index, 0);

    q_ptr->beginInsertRows(localParent, first, last);
    for (int i = first; i <= last; i++) {
        p->m_lChildren << new InternalItem {
            p->m_lChildren.size(),
            InternalItem::Mode::PROXY,
            src,
            p,
            {},
            QStringLiteral("N/A"),
            {}, {}, {}, {}
        };
    }
    q_ptr->endInsertRows();
}

void QMultiModelTreePrivate::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    const auto model = qobject_cast<QAbstractItemModel*>(QObject::sender());
    Q_ASSERT(model);

    slotAddRows(parent, first, last, model);
}

void QMultiModelTreePrivate::slotDataChanged(const QModelIndex& tl, const QModelIndex& br)
{
    if (tl == br && !tl.parent().isValid()) {
        const auto i = q_ptr->mapFromSource(tl);

        Q_ASSERT((!tl.isValid()) || i.isValid());

        Q_EMIT q_ptr->dataChanged(i, i);
    }
    else {
        Q_EMIT q_ptr->dataChanged(q_ptr->mapFromSource(tl), q_ptr->mapFromSource(br));
    }
}

QModelIndex QMultiModelTree::appendModel(QAbstractItemModel* model, const QVariant& id)
{
    if ((!model) || d_ptr->m_hModels[model]) return {};

    beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
    d_ptr->m_hModels[model] = new InternalItem {
        d_ptr->m_lRows.size(),
        InternalItem::Mode::ROOT,
        model,
        Q_NULLPTR,
        {},
        id.canConvert<QString>() ? id.toString() : model->objectName(),
        id, {}, {}, {}
    };
    d_ptr->m_lRows << d_ptr->m_hModels[model];
    endInsertRows();

    d_ptr->slotAddRows({}, 0, model->rowCount(), model);

    //TODO connect to the model row moved/removed/reset
    connect(model, &QAbstractItemModel::rowsInserted,
        d_ptr, &QMultiModelTreePrivate::slotRowsInserted);
    connect(model, &QAbstractItemModel::dataChanged,
        d_ptr, &QMultiModelTreePrivate::slotDataChanged);

    return index(rowCount()-1, 0);
}

bool QMultiModelTree::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    auto idx = index(row, column, parent);

    if (!idx.isValid())
        return false;

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    auto srcIdx = mapToSource(idx);

    return i->m_pModel->canDropMimeData(data, action, srcIdx.row(), srcIdx.column(), srcIdx.parent());
}

bool QMultiModelTree::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    auto idx = index(row, column, parent);

    if (!idx.isValid())
        return false;

    const auto i = static_cast<InternalItem*>(idx.internalPointer());

    auto srcIdx = mapToSource(idx);

    return i->m_pModel->dropMimeData(data, action, srcIdx.row(), srcIdx.column(), srcIdx.parent());
}

QMimeData* QMultiModelTree::mimeData(const QModelIndexList &indexes) const
{
    QModelIndexList newList;

    bool singleModel = true;
    QAbstractItemModel* srcModel = Q_NULLPTR;

    for (auto i : qAsConst(indexes)) {
        const auto srcIdx = mapToSource(i);
        Q_ASSERT(srcIdx.model() != this);

        if (!srcModel)
            srcModel = const_cast<QAbstractItemModel*>(srcIdx.model());
        else if (srcIdx.model() && srcIdx.model() != srcModel) {
            singleModel = false;
            break;
        }

        if (i.isValid()) {
            Q_ASSERT(i.model() == this);
            newList << srcIdx;
        }
    }

    if (singleModel && (!newList.isEmpty()) && srcModel)
        return srcModel->mimeData(newList);

    return QAbstractItemModel::mimeData(indexes);
}

int QMultiModelTree::topLevelIdentifierRole() const
{
    return d_ptr->m_IdRole;
}

void QMultiModelTree::setTopLevelIdentifierRole(int role)
{
    d_ptr->m_HasIdRole = true;
    d_ptr->m_IdRole = role;
}
