#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class ChronoNodePrivate;

class ChronoNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true);


    Q_INVOKABLE explicit ChronoNode(QObject* parent = nullptr);
    virtual ~ChronoNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    ChronoNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChronoNode)

};
Q_DECLARE_METATYPE(ChronoNode*)
