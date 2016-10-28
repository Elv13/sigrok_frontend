#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class HeadNodePrivate;

class HeadNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true);


    Q_INVOKABLE explicit HeadNode(QObject* parent = nullptr);
    virtual ~HeadNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    HeadNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(HeadNode)

};
Q_DECLARE_METATYPE(HeadNode*)
