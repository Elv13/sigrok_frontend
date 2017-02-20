#pragma once

#include "common/abstractnode.h"

#include <QtCore/QObject>

class RemoteControlsPrivate;

class RemoteControls : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("remotecontrols_node", "Control widgets", "", /*Tags:*/ "")
public:

    Q_INVOKABLE explicit RemoteControls(AbstractSession* sess);
    virtual ~RemoteControls();

    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}

    virtual QAbstractItemModel* sourceModel() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual bool createSourceSocket(const QString& name) override;

private:
    RemoteControlsPrivate* d_ptr;
    Q_DECLARE_PRIVATE(RemoteControls)

};
Q_DECLARE_METATYPE(RemoteControls*)
