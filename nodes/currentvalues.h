#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class CurrentValuesPrivate;

class Q_DECL_EXPORT CurrentValues : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("currentvalues_node", "Current Values", "", /*Tags:*/ "")
public:

    Q_INVOKABLE explicit CurrentValues(AbstractSession* sess);
    virtual ~CurrentValues();

    virtual QWidget* widget() const override;
    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}
    virtual QAbstractItemModel* sourceModel() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    CurrentValuesPrivate* d_ptr;
    Q_DECLARE_PRIVATE(CurrentValues)

};
Q_DECLARE_METATYPE(CurrentValues*)
