#include "devicemodel.h"

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "nodes/devicenode.h"

/* Avoid converting string between std and QtCore */
struct QSigRokDevice
{
    QString m_Name;
    std::shared_ptr<sigrok::HardwareDevice> m_pDevice;
    DeviceNode* m_pNode;
};

class DeviceModelPrivate
{
public:
    QVector<QSigRokDevice*> m_lDevices;
    QItemSelectionModel* m_pSelectionModel {nullptr};
    std::shared_ptr<sigrok::Context> m_pContext {nullptr};

    void slotIndexChanged(const QModelIndex& idx, const QModelIndex& previous);
};

void DeviceModelPrivateslotIndexChanged(const QModelIndex& idx, const QModelIndex& previous)
{
    //TODO
}

DeviceModel::DeviceModel() : QAbstractListModel(), d_ptr(new DeviceModelPrivate)
{
}

DeviceModel::~DeviceModel()
{
    delete d_ptr->m_pSelectionModel;
}

DeviceModel* DeviceModel::instance()
{
    static auto d = new DeviceModel();

    static bool isScanning = false;
    if (!isScanning) {\
        isScanning = true;
        d->scan();
    }

    return d;
}

QItemSelectionModel* DeviceModel::selectionModel() const
{
    if (!d_ptr->m_pSelectionModel) {
        d_ptr->m_pSelectionModel = new QItemSelectionModel(
            const_cast<DeviceModel*>(this)
        );

        if (rowCount())
            d_ptr->m_pSelectionModel->setCurrentIndex(
                index(d_ptr->m_lDevices.size()-1,0),
                QItemSelectionModel::ClearAndSelect
            );

//         connect(
//             d_ptr->m_pSelectionModel,
//             &QItemSelectionModel::currentChanged,
//             d_ptr,
//             &DeviceModelPrivate::slotIndexChanged
//         );
    }

    return d_ptr->m_pSelectionModel;
}

std::shared_ptr<sigrok::Context> DeviceModel::context() const
{
    if (!d_ptr->m_pContext) {
        d_ptr->m_pContext = sigrok::Context::create();
    }

    return d_ptr->m_pContext;
}

std::shared_ptr<sigrok::HardwareDevice> DeviceModel::currentDevice() const
{
    if (selectionModel()->currentIndex().isValid()) {
        const int row = d_ptr->m_pSelectionModel->currentIndex().row();
        qDebug() << row;
        return d_ptr->m_lDevices[row]->m_pDevice;
    }

    return nullptr;
}

void DeviceModel::scan()
{
    beginResetModel();
    d_ptr->m_lDevices.clear();
    endResetModel();

    for (const auto& pair : context()->drivers()) {
        QString name = QString::fromStdString(pair.first);
        auto drv = pair.second;

        const auto devs = drv->scan();

        for (const auto& dev : devs) {
            beginInsertRows(QModelIndex(), d_ptr->m_lDevices.size(), d_ptr->m_lDevices.size());
            d_ptr->m_lDevices << new QSigRokDevice {
                name, dev, nullptr
            };
            endInsertRows();
        }

    }
}

int DeviceModel::rowCount(const QModelIndex& parent) const
{
    return parent.isValid()?0:d_ptr->m_lDevices.size();
}

QVariant DeviceModel::data(const QModelIndex& idx, int role) const
{
    if (!idx.isValid()) return {};

    switch (role) {
        case Qt::DisplayRole:
            return d_ptr->m_lDevices[idx.row()]->m_Name;
        case (int) DeviceModel::Role::DEVICE_NODE:
            const auto n = d_ptr->m_lDevices[idx.row()];
            if (!n->m_pNode) {
                n->m_pNode = new DeviceNode(
                    n->m_Name,
                    n->m_pDevice,
                    const_cast<DeviceModel*>(this)
                );
            }

            return QVariant::fromValue(n->m_pNode);
    }

    return {};
}
