#include "statusbar.h"

#include "ui_statusbar.h"

class StatusBar2Private
{
public:
    Ui_StatusBar* ui {new Ui_StatusBar};
};

StatusBar2::StatusBar2(QWidget* parent) : QStatusBar(parent),
    d_ptr(new StatusBar2Private())
{
    auto w = new QWidget(this);

    d_ptr->ui->setupUi(w);

    addPermanentWidget(w);
}

StatusBar2::~StatusBar2()
{
    
}
