#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class ColorNodePrivate;

class ColorNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true);

    Q_INVOKABLE explicit ColorNode(QObject* parent = nullptr);
    virtual ~ColorNode();

    virtual QString title() const;

    virtual QWidget* widget() const;

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void test2Changed(bool);
    void filteredModelChanged(); //dummy

private:
    ColorNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColorNode)

};
Q_DECLARE_METATYPE(ColorNode*)
