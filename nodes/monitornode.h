#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class MonitorNodePrivate;

class Q_DECL_EXPORT MonitorNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("monitor_node", "Value monitor", "", /*Tags:*/ "inline")
public:
    Q_PROPERTY(QVariant value WRITE setValue USER true)

    Q_INVOKABLE explicit MonitorNode(AbstractSession* sess);
    virtual ~MonitorNode();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    void setValue(const QVariant& v);
    QVariant value() const;

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    MonitorNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MonitorNode)

};
Q_DECLARE_METATYPE(MonitorNode*)
