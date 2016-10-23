#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class ChartNodePrivate;

class ChartNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_INVOKABLE explicit ChartNode(QObject* parent = nullptr);
    virtual ~ChartNode();

    virtual QString title() const;
    virtual QWidget* widget() const;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

//     virtual void connectTo(AbstractNode* other, QAbstractItemModel* model, int col) override;

private:
    ChartNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(ChartNode)

};
Q_DECLARE_METATYPE(ChartNode*)
