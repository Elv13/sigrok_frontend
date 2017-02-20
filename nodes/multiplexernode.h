#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class MultiplexerNodePrivate;

class Q_DECL_EXPORT MultiplexerNode : public AbstractNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit MultiplexerNode(AbstractSession* sess);
    virtual ~MultiplexerNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual Mode mode() const override {return AbstractNode::Mode::MODEL;}

    virtual void write(QJsonObject &parent) const override;

    virtual QAbstractItemModel* sourceModel() const override;

    virtual bool createSinkSocket(const QString& name) override;
    virtual bool createSourceSocket(const QString& name) override;

    virtual QStringList searchTags() const override;
// Q_SIGNALS:
//     void modelChangedCopy(QAbstractItemModel* newModel, QAbstractItemModel* old);

private:
    MultiplexerNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MultiplexerNode)

};
Q_DECLARE_METATYPE(MultiplexerNode*)
