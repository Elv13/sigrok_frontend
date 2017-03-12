#include "xmlimport.h"

#include <QtCore/QDebug>
#include <QtWidgets/QTableView>
#include "ui_xmlimport.h"

#include "proxies/xmlschemabuildermodel.h"

XmlImport::XmlImport(QWidget* parent) : QDialog(parent)
{
    Ui_XmlImport ui;
    ui.setupUi(this);

    auto m = new XmlSchemaBuilderModel;
    Q_ASSERT(!m->index(0,0).parent().isValid());
    Q_ASSERT(m->index(0,0).isValid());
    Q_ASSERT(m->rowCount() == 1);
    qDebug() << "\n\nFOO" << m->rowCount() << m->rowCount(m->index(0,0)) << m->index(0,0).data();
    ui.treeView->setModel(m);
}

XmlImport::~XmlImport()
{

}
