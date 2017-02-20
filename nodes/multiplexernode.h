#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class MultiplexerNodePrivate;

class Q_DECL_EXPORT MultiplexerNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("multiplexer_node", "Multiplexer", "", /*Tags:*/ "divider")
public:
    Q_INVOKABLE explicit MultiplexerNode(AbstractSession* sess);
    virtual ~MultiplexerNode();

    virtual QWidget* widget() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}

    virtual void write(QJsonObject &parent) const override;

    virtual QAbstractItemModel* sourceModel() const override;

    virtual bool createSinkSocket(const QString& name) override;
    virtual bool createSourceSocket(const QString& name) override;

// Q_SIGNALS:
//     void modelChangedCopy(QAbstractItemModel* newModel, QAbstractItemModel* old);

private:
    MultiplexerNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MultiplexerNode)

};
Q_DECLARE_METATYPE(MultiplexerNode*)
