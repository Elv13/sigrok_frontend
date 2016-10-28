#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TailNodePrivate;

class TailNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true);


    Q_INVOKABLE explicit TailNode(QObject* parent = nullptr);
    virtual ~TailNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    TailNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TailNode)

};
Q_DECLARE_METATYPE(TailNode*)
