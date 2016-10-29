#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class CurrentValuesPrivate;

class CurrentValues : public ProxyNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit CurrentValues(QObject* parent = nullptr);
    virtual ~CurrentValues();

    virtual QString title() const;
    virtual QWidget* widget() const;
    virtual Mode mode() const {return AbstractNode::Mode::MODEL;}
    virtual QAbstractItemModel* sourceModel() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    CurrentValuesPrivate* d_ptr;
    Q_DECLARE_PRIVATE(CurrentValues)

};
Q_DECLARE_METATYPE(CurrentValues*)
