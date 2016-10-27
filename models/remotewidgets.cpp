#include "remotewidgets.h"

#include <QtCore/QDebug>

#include <QRemoteObjectHost>
#include "../remotemanager.h"

#include "remotewidgets_source.h"

struct DataHolder
{
    QString    name {QStringLiteral("[ADD NEW]")};
    QByteArray property;
    QObject*   object {nullptr};
    int        id;
};

class RemoteWidgetsExport final : public RemoteWidgetsSimpleSource
{
    Q_OBJECT
public:
    explicit RemoteWidgetsExport(QObject* parent = nullptr) : RemoteWidgetsSimpleSource(parent) {}

    RemoteWidgetsPrivate* d_ptr;

public Q_SLOTS:
    void activated(int index, QVariant value);
};

class RemoteWidgetsPrivate
{
public:
    RemoteWidgetsExport m_Exported;
    int m_MaxId {0};
    QVector<DataHolder*> m_lRows {{new DataHolder}};
};

RemoteWidgets::RemoteWidgets(QObject* o) : QAbstractListModel(o),
    d_ptr(new RemoteWidgetsPrivate)
{
    auto host = RemoteManager::instance()->host();
    d_ptr->m_Exported.d_ptr = d_ptr;

    if (!host->enableRemoting(&d_ptr->m_Exported)) {
        qWarning() << "Remote widgets sharing failed";
    }
}

RemoteWidgets::~RemoteWidgets()
{
    
}

void RemoteWidgetsExport::activated(int index, QVariant value)
{

    if (index < d_ptr->m_lRows.size()) {
        auto dh = d_ptr->m_lRows[index];

        dh->object->setProperty(dh->property, value);
    }
}

QVariant RemoteWidgets::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    const auto dh = d_ptr->m_lRows[idx.row()];

    switch(role) {
        case Qt::DisplayRole:
            return dh->name;
    }

    return {};
}

bool RemoteWidgets::setData(const QModelIndex& idx, const QVariant& value, int role)
{

    if (idx.isValid() && role == 999 && value.canConvert<QObject*>()) {
        d_ptr->m_lRows[idx.row()]->object = qvariant_cast<QObject*>(value);
        d_ptr->m_lRows[idx.row()]->name = d_ptr->m_lRows[idx.row()]->object->objectName();
        Q_EMIT dataChanged(idx, idx);

        // Always add more rows when the last once is used
        if (idx.row() == d_ptr->m_lRows.size() -1) {
            beginInsertRows({}, d_ptr->m_lRows.size(), d_ptr->m_lRows.size());
            d_ptr->m_lRows << new DataHolder();
            d_ptr->m_lRows.last()->id = d_ptr->m_MaxId++;
            endInsertRows();
        }


        return true;
    }
    if (idx.isValid() && role == 998 && d_ptr->m_lRows[idx.row()]->object) {
        d_ptr->m_lRows[idx.row()]->property =  qvariant_cast<QByteArray>(value);
        d_ptr->m_lRows[idx.row()]->name += "."+value.toString();
        Q_EMIT dataChanged(idx, idx);

        // Add it to the remote control
        auto names = d_ptr->m_Exported.names();
        auto types = d_ptr->m_Exported.types();
        names << value.toString();
        d_ptr->m_Exported.setNames(names);

        return true;
    }

    return false;
}

int RemoteWidgets::rowCount(const QModelIndex& parent) const
{
    return parent.isValid() ? 0 : d_ptr->m_lRows.size();
}

#include <remotewidgets.moc>
