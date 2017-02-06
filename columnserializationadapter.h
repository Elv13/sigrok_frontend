#pragma once

#include <QtCore/QObject>
#include <QtCore/QHash>

class QAbstractItemModel;

class ColumnSerializationAdapter : public QObject
{
    Q_OBJECT
public:
    explicit ColumnSerializationAdapter(QAbstractItemModel* m, const QList<int>& cols, QObject* p);

    void write(QJsonObject &parent) const;
    void read(const QJsonObject &parent);

private:
    QHash<int, QHash<QString, bool> > m_hhActivated;
    QAbstractItemModel* m_pModel;
    QList<int> m_lColumns;

private Q_SLOTS:
    void slotRowsInserted(const QModelIndex& parent, int first, int last);
    void slotReload();
};

