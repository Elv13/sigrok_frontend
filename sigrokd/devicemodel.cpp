#include "devicemodel.h"

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include "sigrokdevice.h"

#include <libsigrokcxx/libsigrokcxx.hpp>

// #include "nodes/devicenode.h"

class DeviceModelPrivate
{
public:
    QVector<SigrokDevice*> m_lDevices;
    QItemSelectionModel* m_pSelectionModel {nullptr};
    std::shared_ptr<sigrok::Context> m_pContext {nullptr};

    void slotIndexChanged(const QModelIndex& idx, const QModelIndex& previous);
};

DeviceModel::DeviceModel() : QAbstractListModel(), d_ptr(new DeviceModelPrivate)
{
}

DeviceModel::~DeviceModel()
{
    delete d_ptr->m_pSelectionModel;
    delete d_ptr;
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
        return d_ptr->m_lDevices[row]->native();
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
            d_ptr->m_lDevices << new SigrokDevice( dev, name, this );
            endInsertRows();
        }

    }
}

Qt::ItemFlags DeviceModel::flags(const QModelIndex &idx) const
{
    return idx.isValid() ?
        Qt::ItemIsEnabled | Qt::ItemIsSelectable |Qt::ItemIsDragEnabled :
        Qt::NoItemFlags;
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
            return d_ptr->m_lDevices[idx.row()]->name();
        case Qt::EditRole:
        case (int) DeviceModel::Role::DEVICE:
            const auto n = d_ptr->m_lDevices[idx.row()];
//             if (!n->m_pNode) {
//                 n->m_pNode = new DeviceNode(
// //                     n->m_Name,
// //                     n->m_pDevice,
//                     const_cast<DeviceModel*>(this)
//                 );
//                 n->m_pNode->setDevice(n->m_pDevice);
//             }
            return QVariant::fromValue(n);
    }

    return {};
}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    static QHash<int, QByteArray> ret;

    if (ret.isEmpty()) {
        ret[ (int) Role::DEVICE ] = "device";
    }

    return ret;
}

QVariant DeviceModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ((!section) && orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return tr("Devices");

    return {};
}
