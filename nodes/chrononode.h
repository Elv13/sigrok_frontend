#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class ChronoNodePrivate;

class Q_DECL_EXPORT ChronoNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)


    Q_INVOKABLE explicit ChronoNode(AbstractSession* sess);
    virtual ~ChronoNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    bool hasDeltaColumn    () const;
    bool hasMsEpochColumn  () const;
    bool hasTimestampColumn() const;
    bool hasDateTimeColumn () const;

    QAbstractItemModel* filteredModel() const;

public Q_SLOTS:
    void enableDelta(bool);
    void enableMsEpoch(bool);
    void enableTimestamp(bool);
    void enableDateTime(bool);

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    ChronoNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChronoNode)

};
Q_DECLARE_METATYPE(ChronoNode*)
