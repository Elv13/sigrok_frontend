#include "timernode.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include "widgets/timer.h"

class TimerNodePrivate final : public QObject
{
public:
    int m_Ms {0};
    QTimer m_Timer {this};
    Timer m_Widget;

    TimerNode* q_ptr;

public Q_SLOTS:
    void timeout();
};

TimerNode::TimerNode(QObject* parent) : AbstractNode(parent), d_ptr(new TimerNodePrivate())
{
    d_ptr->q_ptr = this;
    QObject::connect(&d_ptr->m_Widget, &Timer::secondChanged, this, &TimerNode::setSeconds);
    QObject::connect(&d_ptr->m_Widget, &Timer::millisecondChanged, this, &TimerNode::setMilliseconds);
    connect(&d_ptr->m_Widget, &Timer::activated, this, &TimerNode::setActive);
    connect(&d_ptr->m_Timer, &QTimer::timeout, d_ptr, &TimerNodePrivate::timeout);

}

TimerNode::~TimerNode()
{
    delete d_ptr;
}

QString TimerNode::title() const
{
    return "Timer";
}

QString TimerNode::id() const
{
    return QStringLiteral("timer_node");
}

void TimerNode::write(QJsonObject &parent) const
{
    AbstractNode::write(parent);
}

QWidget* TimerNode::widget() const
{
    return &d_ptr->m_Widget;
}

int TimerNode::milliseconds() const
{
    return d_ptr->m_Ms;
}

int TimerNode::seconds() const
{
    return d_ptr->m_Ms/1000;
}

bool TimerNode::isActive() const
{
    return d_ptr->m_Timer.isActive();
}

void TimerNodePrivate::timeout()
{
    Q_EMIT q_ptr->tick(true);
}

void TimerNode::setMilliseconds(int value)
{
    d_ptr->m_Ms = value;
    d_ptr->m_Timer.setInterval(d_ptr->m_Ms);
}

void TimerNode::setSeconds(int value)
{
    d_ptr->m_Ms = value * 1000;
    d_ptr->m_Timer.setInterval(d_ptr->m_Ms);
}

void TimerNode::setActive(bool value)
{

    if (value)
        d_ptr->m_Timer.start();
    else
        d_ptr->m_Timer.stop();

    d_ptr->m_Widget.setActive(isActive());

    Q_EMIT activated(isActive());
}
