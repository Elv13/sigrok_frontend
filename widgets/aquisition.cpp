#include "aquisition.h"

// #include "ratesink.h"
// #include "aquisitionmodel.h"

#include <QtCore/QDebug>

#include "ui_aquisition.h"

Aquisition::Aquisition(bool showAquired, QWidget* parent) : QWidget(parent)
{
    Ui_Aquisition aq;
    aq.setupUi(this);

    if (!showAquired)
        aq.m_pAquire->setHidden(true);

    m_pAcqB = aq.m_pAquire;
}

Aquisition::~Aquisition()
{

}

void Aquisition::setShowAcquire(bool v)
{
    m_pAcqB->setHidden(v);
}

void Aquisition::slotStarted()
{
    Q_EMIT this->started();
}

void Aquisition::slotStopped()
{
    Q_EMIT stopped();
}

void Aquisition::slotCleared()
{
    Q_EMIT cleared();
}

void Aquisition::slotAquire()
{
    Q_EMIT aquired();
}
