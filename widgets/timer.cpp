#include "timer.h"

#include "ui_timer.h"

#include "nodes/timernode.h"

Timer::Timer(const TimerNode* n, QWidget* parent) : QWidget(parent)
{
    Ui_Timer ui;
    ui.setupUi(this);
    m_pCheck = ui.m_pActivated;

    ui.m_pMilliSeconds->setValue(n->milliseconds());
    ui.m_pSeconds->setValue(n->seconds());
    ui.m_pActivated->setChecked(n->isActive());
}

Timer::~Timer()
{

}

void Timer::setActive(bool a)
{
    m_pCheck->setChecked(a);
}

void Timer::slotSeconds(int v)
{
    Q_EMIT secondChanged(v);
}

void Timer::slotsMs(int v)
{
    Q_EMIT millisecondChanged(v);
}

void Timer::slotActivated(bool a)
{
    Q_EMIT activated(a);
}
