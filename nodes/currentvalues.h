#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class CurrentValuesPrivate;

class Q_DECL_EXPORT CurrentValues : public ProxyNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit CurrentValues(QObject* parent = nullptr);
    virtual ~CurrentValues();

    virtual QString title() const override;
    virtual QWidget* widget() const override;
    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}
    virtual QAbstractItemModel* sourceModel() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    CurrentValuesPrivate* d_ptr;
    Q_DECLARE_PRIVATE(CurrentValues)

};
Q_DECLARE_METATYPE(CurrentValues*)
