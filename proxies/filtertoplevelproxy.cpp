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
#include "filtertoplevelproxy.h"

class FilterTopLevelProxyPrivate final : public QObject
{
    Q_OBJECT
public:
    explicit FilterTopLevelProxyPrivate(FilterTopLevelProxy* parent);

    bool m_RecursionLock {false}; //Avoid infinite loop (not thread safe)

public Q_SLOTS:
    void changeParent(const QModelIndex& parent, int start, int end );

private:
    FilterTopLevelProxy* q_ptr;
};

FilterTopLevelProxyPrivate::FilterTopLevelProxyPrivate(FilterTopLevelProxy* parent) : QObject(parent),
q_ptr(parent)
{
}


FilterTopLevelProxy::FilterTopLevelProxy(QObject* parent) : QSortFilterProxyModel(parent),
d_ptr(new FilterTopLevelProxyPrivate(this))
{
}

FilterTopLevelProxy::~FilterTopLevelProxy()
{
   delete d_ptr;
}

void FilterTopLevelProxy::setSourceModel(QAbstractItemModel* source)
{
    if (sourceModel()) {
        disconnect(sourceModel(), &QAbstractItemModel::rowsRemoved , d_ptr, &FilterTopLevelProxyPrivate::changeParent);
        disconnect(sourceModel(), &QAbstractItemModel::rowsInserted, d_ptr, &FilterTopLevelProxyPrivate::changeParent);
    }

    QSortFilterProxyModel::setSourceModel(source);

    if (source) {
        connect(source, &QAbstractItemModel::rowsRemoved , d_ptr, &FilterTopLevelProxyPrivate::changeParent);
        connect(source, &QAbstractItemModel::rowsInserted, d_ptr, &FilterTopLevelProxyPrivate::changeParent);
    }
}
#include <QtCore/QDebug>
bool FilterTopLevelProxy::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (source_row >= 0 && (!d_ptr->m_RecursionLock) && !source_parent.isValid()) {
        qDebug() << "GET" << source_row;
        return sourceModel()->index(0, 0, sourceModel()->index(source_row,0)).isValid();
    }

    return true;
}

///This proxy doesn't sort anything
void FilterTopLevelProxy::sort ( int column, Qt::SortOrder order)
{
    sourceModel()->sort(column, order);
}

///Force filterAcceptsRow to be called when children cound change
void FilterTopLevelProxyPrivate::changeParent(const QModelIndex& parent, int start, int end )
{
    Q_UNUSED(end)

    /// (performance) Only bother when the first child is involved
    if ((!start) && parent.isValid() && !parent.parent().isValid()) {
//         m_RecursionLock = true;
        qDebug() << "ADD CHILD";
        Q_EMIT q_ptr->sourceModel()->dataChanged(parent,parent);
//         m_RecursionLock = false;
    }
}

#include "filtertoplevelproxy.moc"
