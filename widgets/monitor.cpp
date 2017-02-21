#include "monitor.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_monitor.h"

Monitor::Monitor(QWidget* parent) : QWidget(parent)
{
    Ui_Monitor ui;
    ui.setupUi(this);

    m_pName    = ui.m_pName;
    m_pType    = ui.m_pType;
    m_pTime    = ui.m_pTime;
    m_pContent = ui.m_pContent;

    m_pName->setDisabled(true);
}

Monitor::~Monitor()
{

}

void Monitor::setName(const QString& s)
{
    m_pName->setText(s);
}

void Monitor::setType(const QString& s)
{
    m_pType->setText(s);
}

void Monitor::setTime(const QString& s)
{
    m_pTime->setText(s);
}

void Monitor::setContent(const QString& s)
{
    m_pContent->setText(s);
}
