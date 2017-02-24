#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class ScaleNodePrivate;

class Q_DECL_EXPORT ScaleNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("scale_node", "Scale", "", /*Tags:*/
        "progressbar",
        "range",
        "rate"
    )
public:
    Q_PROPERTY(qreal upperLimit WRITE setUpperLimit USER true)
    Q_PROPERTY(qreal upperLower WRITE setLowerLimit USER true)

    Q_INVOKABLE explicit ScaleNode(AbstractSession* sess);
    virtual ~ScaleNode();

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QString remoteWidgetType() const override;

    virtual QWidget* widget() const override;

    void setLowerLimit(qreal v);
    void setUpperLimit(qreal v);

private:
    ScaleNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ScaleNode)

};
Q_DECLARE_METATYPE(ScaleNode*)
