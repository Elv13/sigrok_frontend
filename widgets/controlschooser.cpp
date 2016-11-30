#include "controlschooser.h"

// #include "ratesink.h"
#include "ui_controlschooser.h"

#include "common/remotemanager.h"

#include <QtCore/QDebug>

ControlsChooser::ControlsChooser(QWidget* parent) : QWidget(parent)
{
    Ui_ControlChooser ui;
    ui.setupUi(this);

    m_pComboBox = ui.comboBox;

    m_ObjList = new RemoteObjectList{"RemoteWidgetsClient" , this};

    ui.comboBox->setModel(m_ObjList);
}

ControlsChooser::~ControlsChooser()
{

}

void ControlsChooser::slotIndexChanged(int)
{
    Q_EMIT currentModelChanged(currentModel());
}

QAbstractItemModel* ControlsChooser::currentModel() const
{
    const int idx = m_pComboBox->currentIndex();

    return m_ObjList->getModel(m_ObjList->index(idx, 0));
}
