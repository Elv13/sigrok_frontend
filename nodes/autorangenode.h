#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class AutoRangeNodePrivate;

class Q_DECL_EXPORT AutoRangeNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("autorange_node", "Auto Range", "", /*Tags:*/
        "unit",
        "magnitude",
        "limits"
    )
public:
    Q_PROPERTY(qreal minimum READ minimum NOTIFY minimumChanged USER true)
    Q_PROPERTY(qreal maximum READ maximum NOTIFY maximumChanged USER true)

    Q_INVOKABLE explicit AutoRangeNode(AbstractSession* sess);
    virtual ~AutoRangeNode();

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QWidget* widget() const override;

    qreal minimum() const;
    qreal maximum() const;

Q_SIGNALS:
    void minimumChanged(qreal minimum);
    void maximumChanged(qreal maximum);

private:
    AutoRangeNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(AutoRangeNode)

};
Q_DECLARE_METATYPE(AutoRangeNode*)
