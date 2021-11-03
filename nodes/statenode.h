#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class StateNodePrivate;

class Q_DECL_EXPORT StateNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("state_node", "State machine", "", /*Tags:*/ "automata")
public:
    Q_INVOKABLE explicit StateNode(AbstractSession* sess);
    virtual ~StateNode();

    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}

    virtual void write(QJsonObject &parent) const override;

    virtual QAbstractItemModel* sourceModel() const override;

    virtual bool createSinkSocket(const QString& name) override;
    virtual bool createSourceSocket(const QString& name) override;

// Q_SIGNALS:
//     void modelChangedCopy(QAbstractItemModel* newModel, QAbstractItemModel* old);

private:
    StateNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(StateNode)

};
Q_DECLARE_METATYPE(StateNode*)
