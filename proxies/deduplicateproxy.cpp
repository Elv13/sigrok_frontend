/****************************************************************************
 *   Copyright (C) 2015 by Emmanuel Lepage Vallee                           *
 *   Author : Emmanuel Lepage Vallee <elv1313@gmail.com>                    *
 *                                                                          *
 *   This library is free software; you can redistribute it and/or          *
 *   modify it under the terms of the GNU Lesser General Public             *
 *   License as published by the Free Software Foundation; either           *
 *   version 2.1 of the License, or (at your option) any later version.     *
 *                                                                          *
 *   This library is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU      *
 *   Lesser General Public License for more details.                        *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "deduplicateproxy.h"

#include <QtCore/QDebug>

typedef std::function<bool(const QModelIndex&, const QModelIndex&)> Comparator;

class DeduplicateProxyPrivate final
{
public:
    // Attributes
    bool                   m_AddChildren    { false                         };
    Comparator             m_fCompare       {                               };
    DeduplicateProxy::Mode m_Mode           { DeduplicateProxy::Mode::ROLE  };
    bool                   m_isSrcProxy     { false                         };
    QVector<int>           m_lOtherRoles    {                               };
    int                    m_HiddenRoleId   { -1                            };
    bool                   m_isHiddenRoleSet{ false                         };
    QString                m_HiddenRoleName {                               };
    bool                   m_AddExtraColumn { false                         };
    QVariant               m_RoleThreshold  {                               };
    QVariant               m_ExtraColumnName{                               };

    bool acceptRow(const QVariant& source, const QVariant& other) const;
    inline bool isExtraColumn(const QModelIndex& idx) const;
};

DeduplicateProxy::DeduplicateProxy(QObject* parent) : QSortFilterProxyModel(parent),
d_ptr(new DeduplicateProxyPrivate())
{
}

DeduplicateProxy::~DeduplicateProxy()
{
    delete d_ptr;
}

void DeduplicateProxy::setComparator(const Comparator& f)
{
    d_ptr->m_Mode = DeduplicateProxy::Mode::FUNCTION;
    d_ptr->m_fCompare = f;

    invalidateFilter();
}

/**
 * When set and the model in "role" mode (rather than functional), this value
 * will be used when comparing the values.
 * 
 * Without this, the result is using the `operator==` of the QVaraint. When set,
 * it will be computer with 
 */
void DeduplicateProxy::setThreshold(const QVariant& threshold)
{
    d_ptr->m_RoleThreshold = threshold;
    invalidateFilter();

    Q_EMIT thresholdChanged(threshold);
}

/**
 * Add an extra column with the number of occurences
 */
void DeduplicateProxy::setExtraColumn(bool value)
{
    if (d_ptr->m_AddExtraColumn == value)
        return;

    const int cc = columnCount();

    if ((!d_ptr->m_AddExtraColumn) && value) {
        beginInsertColumns({}, cc, cc);
        d_ptr->m_AddExtraColumn = value;
        endInsertColumns();
    }
    else {
        beginRemoveColumns({}, cc-1, cc-1);
        d_ptr->m_AddExtraColumn = value;
        endRemoveColumns();
    }
}

/**
 * Useful when there is a main filter role, but some items need to be kept.
 *
 * If any role added to this list is different, the index wont be hidden
 *
 * For example using this proxy to create instant messaging thread if they
 * come from the same person, but also create different threads if the date
 * differ.
 */
void DeduplicateProxy::addFilterRole(int role)
{
    d_ptr->m_lOtherRoles << role;

    invalidateFilter();
}

QVector<int> DeduplicateProxy::filterRoles() const
{
    return d_ptr->m_lOtherRoles;// + filterRole();
}

/**
   * Add rejected items as children of the non-rejected matching item
   *
   * @warning This is ignored for indexes that already have children
   */
void DeduplicateProxy::setAddChildren(bool add)
{
    d_ptr->m_AddChildren = add;
}

bool DeduplicateProxy::areChildrenAdded() const
{
    return d_ptr->m_AddChildren;
}

Comparator DeduplicateProxy::comparator() const
{
    return d_ptr->m_fCompare;
}

int DeduplicateProxy::hiddenCountRole() const
{
    return d_ptr->m_HiddenRoleId;
}

QVariant DeduplicateProxy::threshold() const
{
    return d_ptr->m_RoleThreshold;
}

bool DeduplicateProxy::hasExtraColumn() const
{
    return d_ptr->m_AddExtraColumn;
}

bool DeduplicateProxyPrivate::acceptRow(const QVariant& source, const QVariant& other) const
{
    if (m_RoleThreshold.isValid()) {
        //FIXME There is no generic QVariant +/-/*// operators
        double v1(source.toDouble()), v2(other.toDouble());

        const auto diff = v1 > v2 ? v1 - v2 : v2 - v1;

        return m_RoleThreshold < diff;
    }

    return source != other;
}

/**
 * Check if the source_row index is identical to the previous index
 */
bool DeduplicateProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    // Get both source QModelIndex and compare them
    if (source_row > 0) {
        const QModelIndex idx = sourceModel()->index(source_row, filterKeyColumn(), source_parent);

        // It is correct to assume sourceModelIndex - 1 will be part of the same group
        if (idx.row() > 0) {
            const QModelIndex sibling = sourceModel()->index(idx.row() - 1, idx.column(), idx.parent());

            bool accept = false;

            // Allow to filter with more than one role, apply only if they exist
            Q_FOREACH(const int r, d_ptr->m_lOtherRoles) {
                const QVariant v1 = idx.data    (r);
                const QVariant v2 = sibling.data(r);

                // Yes, this is an "=", not "=="
                if ((accept = !(v1.isValid() && v2.isValid() && (!d_ptr->acceptRow(v1, v2)))))
                    break;
            }

            const auto ret = accept || d_ptr->acceptRow(idx.data(filterRole()),sibling.data(filterRole()));

            if ((!ret) && d_ptr->m_AddExtraColumn) {
                const auto counter = mapFromSource(sibling).sibling(
                    sibling.row(),
                    sourceModel()->columnCount(source_parent)
                );
                Q_EMIT const_cast<DeduplicateProxy*>(this)->dataChanged(counter, counter);
            }

            return ret;
        }
    }

    return true;
}

//TODO support for columns too

///Return the number of hidden siblings of idx
int DeduplicateProxy::hiddenSiblingCount(const QModelIndex& idx) const
{
    if ((!sourceModel()) || (!idx.isValid()) || idx.model() != this)
        return 0;

    QModelIndex i = idx;

    //FIXME can't use parent or sibling, so this doesn't work on tree
    if (d_ptr->isExtraColumn(i))
        i = index(i.row(), 0);

    const QModelIndex nextIdx = index(i.row()+1, i.column(), i.parent());

    const QModelIndex srcIdx  = mapToSource(i);

    if (!nextIdx.isValid())
        return sourceModel()->rowCount(srcIdx.parent()) - srcIdx.row();

    const QModelIndex srcNextIdx = mapToSource(nextIdx);

    return srcNextIdx.row() - srcIdx.row();
}

/**
 * Return the list of source model QModelIndex that have been hidden
 */
QList<QModelIndex> DeduplicateProxy::hiddenSiblings(const QModelIndex& idx) const
{
   const int count = hiddenSiblingCount(idx);

   if (!count)
      return {};

   QList<QModelIndex> ret;

   const int         parentRow    = mapToSource(idx).row();
   const QModelIndex srcParentIdx = mapToSource(idx).parent();

   for (int i = 0; i < count; i++)
      ret << sourceModel()->index(parentRow + count, idx.column(), srcParentIdx);

   return ret;
}

/**
 * Add a model data role to report the number of hidden siblings
 */
void DeduplicateProxy::setHiddenCountRole(int role, const QString& name)
{
    d_ptr->m_isHiddenRoleSet = true;
    d_ptr->m_HiddenRoleId    = role;
    d_ptr->m_HiddenRoleName  = name;
}

void DeduplicateProxy::setExtraColumnName(const QString& name)
{
    d_ptr->m_ExtraColumnName = name;
}

///This proxy doesn't sort anything
void DeduplicateProxy::sort ( int column, Qt::SortOrder order)
{
    sourceModel()->sort(column, order);
}

void DeduplicateProxy::setSourceModel ( QAbstractItemModel * sourceModel )
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    d_ptr->m_isSrcProxy = sourceModel && qobject_cast<QAbstractProxyModel*>(sourceModel);

    //TODO find a way to catch setFilterString to forward it as this proxy
    // is likely in front of another QSortFilterProxyModel re-implement ::invalidateFilter?
}

int DeduplicateProxy::rowCount(const QModelIndex& parent) const
{
    if (!d_ptr->m_AddChildren)
        return QSortFilterProxyModel::rowCount(parent);

    //TODO ::index, ::mapToSource and ::mapFromSource still need to be implemented
    /*int hidden = 0;
    // Handle when the rejected items need to be added as children
    if (d_ptr->m_AddChildren && (hidden = hiddenSiblingCount(parent))) {
        const QModelIndex src = mapToSource(parent);

        // Only add the children if there is none
        int parentRowCount = sourceModel()->rowCount(src);

        if (!parentRowCount)
            return hidden;
    }*/

    return QSortFilterProxyModel::rowCount(parent); //TODO
}

int DeduplicateProxy::columnCount(const QModelIndex& parent) const
{
    return QSortFilterProxyModel::columnCount(parent) + (
        d_ptr->m_AddExtraColumn ? 1 : 0
    );
}

bool DeduplicateProxyPrivate::isExtraColumn(const QModelIndex& idx) const
{
    return idx.isValid() && m_AddExtraColumn && idx.internalId()==(quint64)-999;
}

QModelIndex DeduplicateProxy::index(int row, int column, const QModelIndex &parent) const
{
    if (d_ptr->m_AddExtraColumn && column == columnCount(parent) -1)
        return createIndex(row, column, -999);

    return QSortFilterProxyModel::index(row, column, parent);
}

Qt::ItemFlags DeduplicateProxy::flags(const QModelIndex &idx) const
{
    return d_ptr->isExtraColumn(idx) ? (
        Qt::ItemIsEnabled | Qt::ItemIsSelectable
    ) : QSortFilterProxyModel::flags(idx);
}

QModelIndex DeduplicateProxy::mapToSource(const QModelIndex& proxyIndex) const
{
    // check if it's the last column without creating an infinite loop
    if (d_ptr->isExtraColumn(proxyIndex) && sourceModel())
        return {};

    return QSortFilterProxyModel::mapToSource(proxyIndex);
}

QModelIndex DeduplicateProxy::parent(const QModelIndex& idx) const
{
    // Incorrect, but not doing it will SEGFAULT as the messes with the SortFilter
    // implementation.
    if (d_ptr->isExtraColumn(idx))
        return {};

    return QSortFilterProxyModel::parent(idx);
}

QVariant DeduplicateProxy::headerData(int s, Qt::Orientation o, int role) const
{
    if (o == Qt::Horizontal && d_ptr->m_AddExtraColumn && s == columnCount() -1 && role == Qt::DisplayRole)
        return d_ptr->m_ExtraColumnName;

    return QSortFilterProxyModel::headerData(s, o, role);
}

QVariant DeduplicateProxy::data( const QModelIndex& index, int role ) const
{
    if ((d_ptr->m_isHiddenRoleSet && role == d_ptr->m_HiddenRoleId) ||
      (d_ptr->isExtraColumn(index) && role == Qt::DisplayRole))
        return hiddenSiblingCount(index);

    return QSortFilterProxyModel::data(index, role);
}
