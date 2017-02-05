#include "timernode.h"

#include <QtCore/QTimer>
#include <QtCore/QDebug>

#include "widgets/timer.h"

class TimerNodePrivate final : public QObject
{
    Q_OBJECT
public:
    int m_Ms {1000};
    QTimer m_Timer {this};
    Timer* m_pWidget {Q_NULLPTR};

    TimerNode* q_ptr;

public Q_SLOTS:
    void timeout();
};

TimerNode::TimerNode(AbstractSession* sess) : AbstractNode(sess), d_ptr(new TimerNodePrivate())
{
    d_ptr->q_ptr = this;
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

    parent[ "milliseconds" ] = seconds()*1000+milliseconds();
}

void TimerNode::read(const QJsonObject &parent)
{
    AbstractNode::read(parent);

    setMilliseconds(parent[ "milliseconds" ].toInt()%1000);
    setSeconds     (parent[ "milliseconds" ].toInt()/1000);
}

QWidget* TimerNode::widget() const
{
    if (!d_ptr->m_pWidget) {
        d_ptr->m_pWidget = new Timer(this);

        d_ptr->m_pWidget->blockSignals(true);
        d_ptr->m_pWidget->blockSignals(true);
        d_ptr->m_pWidget->setActive(isActive());
        d_ptr->m_pWidget->blockSignals(false);

        connect(d_ptr->m_pWidget, &Timer::secondChanged, this, &TimerNode::setSeconds);
        connect(d_ptr->m_pWidget, &Timer::millisecondChanged, this, &TimerNode::setMilliseconds);
        connect(d_ptr->m_pWidget, &Timer::activated, this, &TimerNode::setActive);

        connect(this, &TimerNode::secondsChanged, d_ptr->m_pWidget, &Timer::slotSeconds);
        connect(this, &TimerNode::millisecondsChanged, d_ptr->m_pWidget, &Timer::slotsMs);
        connect(this, &TimerNode::activated, d_ptr->m_pWidget, &Timer::slotActivated);
    }

    return d_ptr->m_pWidget;
}

int TimerNode::milliseconds() const
{
    return d_ptr->m_Ms%1000;
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
    if (value%1000 == d_ptr->m_Ms%1000)
        return;

    d_ptr->m_Ms = value;
    d_ptr->m_Timer.setInterval(d_ptr->m_Ms);
    Q_EMIT millisecondsChanged(value);
}

void TimerNode::setSeconds(int value)
{
    if (value == d_ptr->m_Ms/1000)
        return;

    d_ptr->m_Ms = value * 1000 + d_ptr->m_Ms%1000;
    d_ptr->m_Timer.setInterval(d_ptr->m_Ms);
    Q_EMIT secondsChanged(value);
}

void TimerNode::setActive(bool value)
{
    if (value == isActive())
        return;

    if (value)
        d_ptr->m_Timer.start();
    else
        d_ptr->m_Timer.stop();

    if (d_ptr->m_pWidget) {
        d_ptr->m_pWidget->blockSignals(true);
        d_ptr->m_pWidget->blockSignals(true);
        d_ptr->m_pWidget->setActive(isActive());
        d_ptr->m_pWidget->blockSignals(false);
    }

    Q_EMIT activated(isActive());
}

bool TimerNode::dummy() const {return false;}

#include <timernode.moc>

