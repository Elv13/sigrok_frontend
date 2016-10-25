#include "memento.h"

#include <QtCore/QDebug>

Memento::Memento(QWidget* parent) : QWidget(parent)
{
    setupUi(this);
}

Memento::~Memento()
{

}
