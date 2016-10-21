#include "meterproxy.h"

#include <QtCore/QDebug>

class MeterProxyPrivate
{
public:
    QVector<bool> m_lShow;
    int m_Main {-1};
};

MeterProxy::MeterProxy(QObject* parent) : QIdentityProxyModel(parent),
    d_ptr(new MeterProxyPrivate())
{}


int MeterProxy::columnCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : 3;
}

QVariant MeterProxy::headerData(int sec, Qt::Orientation ori, int role) const
{
    if (ori == Qt::Horizontal && role == Qt::DisplayRole) {
        switch(sec) {
            case 0:
                return QStringLiteral("Column");
            case 1:
                return QStringLiteral("V");
            case 2:
                return QStringLiteral("M");
        }
    }

    return QIdentityProxyModel::headerData(sec, ori, role);
}

QModelIndex MeterProxy::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid() && column > 0 && column <= 2) {
        return createIndex(row, column, nullptr);
    }

    return QIdentityProxyModel::index(row, column, parent);
}

Qt::ItemFlags MeterProxy::flags(const QModelIndex &idx) const
{
    Qt::ItemFlags ret = QIdentityProxyModel::flags(idx);

    return ret | Qt::ItemIsUserCheckable;
}

QVariant MeterProxy::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    if (role == Qt::CheckStateRole) {
        switch(idx.column()) {
            case 1:
                d_ptr->m_lShow.resize(rowCount());
                return d_ptr->m_lShow[idx.row()] ? Qt::Checked : Qt::Unchecked;
            case 2:
                return d_ptr->m_Main == idx.row() ? Qt::Checked : Qt::Unchecked;
        }
    }
    else if (idx.column() == 1 && role == Qt::ToolTipRole)
        return QStringLiteral("Display in the meter");
    else if (idx.column() == 2 && role == Qt::ToolTipRole)
        return QStringLiteral("Use as for the main label");
    else if (idx.column() > 0)
        return {};

    return QIdentityProxyModel::data(idx, role);
}

bool MeterProxy::setData(const QModelIndex &i, const QVariant &v, int r)
{
    if (r == Qt::CheckStateRole) {
        switch(i.column()) {
            case 1: {
                const bool isChecked = v == Qt::Checked;

                d_ptr->m_lShow.resize(rowCount());
                d_ptr->m_lShow[i.row()] = isChecked;

                Q_EMIT columnEnabled(i.row(), isChecked);

                return true;
            }
            case 2:
                if (v == Qt::Checked) {
                    const int old = d_ptr->m_Main;

                    d_ptr->m_Main = i.row();

                    // Both can't be true at once
                    d_ptr->m_lShow.resize(rowCount());
                    d_ptr->m_lShow[i.row()] = false;

                    Q_EMIT mainChanged(d_ptr->m_Main);

                    if (old != -1)
                        Q_EMIT dataChanged(index(old,0), index(old,0));

                    return true;
                }
        }
    }

    return false;
}

int MeterProxy::mainColumn() const
{
    return d_ptr->m_Main == -1 ? 0 : d_ptr->m_Main;
}

bool MeterProxy::isColumntSelected(int index) const
{
    if (index < 0 || index >= rowCount()) return false;

    d_ptr->m_lShow.resize(rowCount());
    return d_ptr->m_lShow[index];
}
