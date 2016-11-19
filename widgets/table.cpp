#include "table.h"
#include "ui_table.h"

Table::Table(QWidget* parent) : QWidget(parent)
{
    Ui_Table ui;
    ui.setupUi(this);
}

Table::~Table()
{

}
