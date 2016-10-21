#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class ColumnNodePrivate;

class ColumnNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(bool test2 READ test2 WRITE setTest2 NOTIFY test2Changed USER true);
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true);

    Q_INVOKABLE explicit ColumnNode(QObject* parent = nullptr);
    virtual ~ColumnNode();

    virtual QString title() const;

    virtual QWidget* widget() const;

    bool test2() const;
    void setTest2(bool);

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void test2Changed(bool);
    void filteredModelChanged(); //dummy

private:
    ColumnNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColumnNode)

};
Q_DECLARE_METATYPE(ColumnNode*)
