#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class MultiplexerNodePrivate;

class MultiplexerNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* model2 READ model NOTIFY modelChangedCopy USER true);
    Q_PROPERTY(QAbstractItemModel* model3 READ model NOTIFY modelChangedCopy USER true);
    Q_PROPERTY(QAbstractItemModel* model4 READ model NOTIFY modelChangedCopy USER true);

    Q_INVOKABLE explicit MultiplexerNode(QObject* parent = nullptr);
    virtual ~MultiplexerNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

Q_SIGNALS:
    void modelChangedCopy(QAbstractItemModel* newModel, QAbstractItemModel* old);

private:
    MultiplexerNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MultiplexerNode)

};
Q_DECLARE_METATYPE(MultiplexerNode*)
