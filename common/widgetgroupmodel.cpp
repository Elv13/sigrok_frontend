#include "widgetgroupmodel.h"

#if QT_VERSION < 0x050700
//Q_FOREACH is deprecated and Qt CoW containers are detached on C++11 for loops
template<typename T>
const T& qAsConst(const T& v)
{
    return const_cast<const T&>(v);
}
#endif

class WidgetGroupModelPrivate
{
public:
    QVector<QMainWindow*> m_lGroups;
    QStringList m_lNames;
    QStringList m_lIds;
};

WidgetGroupModel::WidgetGroupModel(QObject* parent) : QAbstractListModel(parent),
    d_ptr(new WidgetGroupModelPrivate())
{

}

WidgetGroupModel::~WidgetGroupModel()
{

}

QVariant WidgetGroupModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid())
        return {};

    if (role == Qt::DisplayRole)
        return d_ptr->m_lNames[idx.row()];

    return {};
}

int WidgetGroupModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lGroups.size();
}

QString WidgetGroupModel::addGroup(QMainWindow* w, const QString& name, const QString& id)
{
    auto i = id;

    for(const auto& ii : qAsConst(d_ptr->m_lIds)) {
        if (ii == i)
            i += QLatin1String("1");
    }

    beginInsertRows({}, d_ptr->m_lGroups.size(), d_ptr->m_lGroups.size());
    d_ptr->m_lGroups << w;
    d_ptr->m_lNames  << name;
    d_ptr->m_lIds    << i;
    endInsertRows();

    return i;
}

QMainWindow* WidgetGroupModel::mainWindow(const QModelIndex& idx) const
{
    if (!idx.isValid())
        return nullptr;

    return d_ptr->m_lGroups[idx.row()];
}
