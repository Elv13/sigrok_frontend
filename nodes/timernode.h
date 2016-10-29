#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class TimerNodePrivate;

class TimerNode : public AbstractNode
{
    Q_OBJECT

public:
    Q_PROPERTY(bool active READ isActive WRITE setActive NOTIFY activated USER true);
    Q_PROPERTY(int milliseconds WRITE setMilliseconds USER true);
    Q_PROPERTY(int seconds WRITE setSeconds USER true);
    Q_PROPERTY(bool tick READ dummy NOTIFY tick USER true);

    Q_INVOKABLE explicit TimerNode(QObject* parent = nullptr);
    virtual ~TimerNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    int milliseconds() const;
    int seconds() const;
    bool isActive() const;


    bool dummy() const {return false;}

    virtual void write(QJsonObject &parent) const override;

public Q_SLOTS:
    void setMilliseconds(int value);
    void setSeconds(int value);
    void setActive(bool value);

Q_SIGNALS:
    void activated(bool);
    void tick(bool);

private:
    TimerNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TimerNode)

};
Q_DECLARE_METATYPE(TimerNode*)