#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class ColorNodePrivate;

class Q_DECL_EXPORT ColorNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("color_node", "Color", "", /*Tags:*/ "range", "alerts")
public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)

    Q_INVOKABLE explicit ColorNode(AbstractSession* sess);
    virtual ~ColorNode();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void test2Changed(bool);
    void filteredModelChanged(); //dummy

private:
    ColorNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ColorNode)

};
Q_DECLARE_METATYPE(ColorNode*)
