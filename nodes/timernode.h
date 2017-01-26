#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class TimerNodePrivate;

class Q_DECL_EXPORT TimerNode : public AbstractNode
{
    Q_OBJECT

public:
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activated USER true)
    Q_PROPERTY(int milliseconds WRITE setMilliseconds USER true)
    Q_PROPERTY(int seconds WRITE setSeconds USER true)
    Q_PROPERTY(bool tick READ dummy NOTIFY tick USER true)

    Q_INVOKABLE explicit TimerNode(AbstractSession* sess);
    virtual ~TimerNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    int milliseconds() const;
    int seconds() const;
    bool isActive() const;

    bool dummy() const;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

public Q_SLOTS:
    void setMilliseconds(int value);
    void setSeconds(int value);
    void setActive(bool value);

Q_SIGNALS:
    void activated(bool);
    void tick(bool);
    void secondsChanged(int);
    void millisecondsChanged(int);

private:
    TimerNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TimerNode)

};
Q_DECLARE_METATYPE(TimerNode*)
