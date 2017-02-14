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

    m_pAcqB   = aq.m_pAquire;
    m_pStatus = aq.m_pStatus;
    m_pTime   = aq.m_pLastSample;
    m_pCount  = aq.m_pCount;
}

Aquisition::~Aquisition()
{

}

void Aquisition::setShowAcquire(bool v)
{
    m_pAcqB->setVisible(v);
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

void Aquisition::setStatus(const QString& st)
{
    m_pStatus->setText(st);
}

void Aquisition::setCount(int count)
{
    m_pCount->setText(QString("(%1)").arg(count));
}

void Aquisition::setLast(const QDateTime& ti)
{
    m_pTime->setText(ti.toString());
}
