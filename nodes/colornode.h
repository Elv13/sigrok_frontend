#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class ColorNodePrivate;

class Q_DECL_EXPORT ColorNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)

    Q_INVOKABLE explicit ColorNode(AbstractSession* sess);
    virtual ~ColorNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    QAbstractItemModel* filteredModel() const;

    virtual QStringList searchTags() const override;

Q_SIGNALS:
    void test2Changed(bool);
    void filteredModelChanged(); //dummy

private:
    ColorNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColorNode)

};
Q_DECLARE_METATYPE(ColorNode*)
