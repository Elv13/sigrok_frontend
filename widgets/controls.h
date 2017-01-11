#pragma once

#include <QtWidgets/QWidget>

class QListView;
class QAbstractItemModel;

class Controls : public QWidget
{
    Q_OBJECT

public:
    explicit Controls(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* m);

    virtual ~Controls();

private Q_SLOTS:
    void slotRowsInserted(const QModelIndex& p, int start, int end);
    void slotModelReset();
    void slotDataChanged(const QModelIndex& tl, const QModelIndex& br);

private:
    QListView* m_pTable;
    QAbstractItemModel* m_pModel {Q_NULLPTR};
};
