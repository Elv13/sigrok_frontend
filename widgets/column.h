#pragma once

#include <QtWidgets/QWidget>

class QListView;
class QAbstractItemModel;

class Column : public QWidget
{
    Q_OBJECT

public:
    explicit Column(QWidget* parent = nullptr);

    void setModel(QAbstractItemModel* m);

    virtual ~Column();

private:
    QListView* m_pListView;
};
