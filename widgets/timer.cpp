#include "timer.h"

#include "ui_timer.h"

Timer::Timer(QWidget* parent) : QWidget(parent)
{
    Ui_Timer ui;
    ui.setupUi(this);
    m_pCheck = ui.toolButton;
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
