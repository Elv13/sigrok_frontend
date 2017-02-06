#include "columnserializationadapter.h"

#include <QtCore/QAbstractItemModel>
#include <QtCore/QJsonArray>
#include <QtCore/QJsonObject>

ColumnSerializationAdapter::ColumnSerializationAdapter(QAbstractItemModel* m, const QList<int>& cols, QObject* p)
     : QObject(p), m_pModel(m), m_lColumns(cols)
{
    connect(m, &QAbstractItemModel::rowsInserted,
        this, &ColumnSerializationAdapter::slotRowsInserted);
    connect(m, &QAbstractItemModel::layoutChanged,
        this, &ColumnSerializationAdapter::slotReload);
    connect(m, &QAbstractItemModel::modelReset,
        this, &ColumnSerializationAdapter::slotReload);
}

void ColumnSerializationAdapter::slotRowsInserted(const QModelIndex& parent, int first, int last)
{
    Q_UNUSED(parent)

    Q_FOREACH(int col, m_lColumns) {
        for (int i = first; i <= last; i++) {
            if (m_hhActivated[col].contains(m_pModel->index(i, 0).data().toString())) {
                m_pModel->setData(m_pModel->index(i, col), Qt::Checked, Qt::CheckStateRole);
            }
        }
    }
}

void ColumnSerializationAdapter::slotReload()
{
    slotRowsInserted({}, 0, m_pModel->rowCount()-1);
}

void ColumnSerializationAdapter::write(QJsonObject &parent) const
{
    Q_FOREACH(int col, m_lColumns) {
        QJsonArray a;

        for (int i=0; i < m_pModel->rowCount(); i++) {
            const auto idx = m_pModel->index(i, col);
            if (idx.data(Qt::CheckStateRole) == Qt::Checked) {
                a.append(m_pModel->index(i, 0).data().toString());
            }
        }
        parent["columns"+QString::number(col)] = a;
    }
}

void ColumnSerializationAdapter::read(const QJsonObject &parent)
{

    Q_FOREACH(int col, m_lColumns) {
        const QJsonArray a = parent["columns"+QString::number(col)].toArray();
        for (int row = 0; row < a.size(); ++row) {
            m_hhActivated[col][a[row].toString()] = true;
        }
    }

    slotReload();
}
