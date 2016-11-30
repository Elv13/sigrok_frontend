#pragma once

#include "../abstractnode.h"

#include <QtCore/QObject>

class RemoteControlsPrivate;

class RemoteControls : public AbstractNode
{
    Q_OBJECT

public:

    Q_INVOKABLE explicit RemoteControls(QObject* parent = nullptr);
    virtual ~RemoteControls();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}

    virtual QString id() const override;

    virtual QAbstractItemModel* sourceModel() const override;

    virtual void write(QJsonObject &parent) const override;

private:
    RemoteControlsPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteControls)

};
Q_DECLARE_METATYPE(RemoteControls*)
