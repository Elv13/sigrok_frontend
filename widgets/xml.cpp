#include "xml.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_xml.h"

#include "xmlimport.h"

Xml::Xml(QWidget* parent) : QWidget(parent)
{
    Ui_Xml ui;
    ui.setupUi(this);
}

Xml::~Xml()
{

}

void Xml::slotSettings()
{
    auto x = new XmlImport();
    x->show();
}

void Xml::slotOpen()
{

}
