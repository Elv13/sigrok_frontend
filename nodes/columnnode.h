#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class ColumnNodePrivate;

class Q_DECL_EXPORT ColumnNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)

    Q_INVOKABLE explicit ColumnNode(QObject* parent = nullptr);
    virtual ~ColumnNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QWidget* widget() const override;

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void test2Changed(bool);
    void filteredModelChanged(); //dummy

private:
    ColumnNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColumnNode)

};
Q_DECLARE_METATYPE(ColumnNode*)
