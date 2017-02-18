#include "acquisition.h"

// #include "ratesink.h"
// #include "acquisitionmodel.h"

#include <QtCore/QDebug>

#include "ui_acquisition.h"

Acquisition::Acquisition(bool showAcquired, QWidget* parent) : QWidget(parent)
{
    Ui_Acquisition aq;
    aq.setupUi(this);

    if (!showAcquired)
        aq.m_pAcquire->setHidden(true);

    m_pAcqB   = aq.m_pAcquire;
    m_pStatus = aq.m_pStatus;
    m_pTime   = aq.m_pLastSample;
    m_pCount  = aq.m_pCount;
}

Acquisition::~Acquisition()
{

}

void Acquisition::setShowAcquire(bool v)
{
    m_pAcqB->setVisible(v);
}

void Acquisition::slotStarted()
{
    Q_EMIT this->started();
}

void Acquisition::slotStopped()
{
    Q_EMIT stopped();
}

void Acquisition::slotCleared()
{
    Q_EMIT cleared();
}

void Acquisition::slotAcquire()
{
    Q_EMIT acquired();
}

void Acquisition::setStatus(const QString& st)
{
    m_pStatus->setText(st);
}

void Acquisition::setCount(int count)
{
    m_pCount->setText(QString("(%1)").arg(count));
}

void Acquisition::setLast(const QDateTime& ti)
{
    m_pTime->setText(ti.toString());
}
