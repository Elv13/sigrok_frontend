#pragma once

#include <QtWidgets/QWidget>

class QComboBox;
class QAbstractItemModel;
class RemoteObjectList;

class ControlsChooser : public QWidget
{
    Q_OBJECT

public:
    explicit ControlsChooser(QWidget* parent = nullptr);

    virtual ~ControlsChooser();

    QAbstractItemModel* currentModel() const;

Q_SIGNALS:
    void currentModelChanged(QAbstractItemModel* m);

private Q_SLOTS:
    void slotIndexChanged(int i);

private:
    QComboBox* m_pComboBox;
    RemoteObjectList* m_ObjList;
};
