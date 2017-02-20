#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TailNodePrivate;

class Q_DECL_EXPORT TailNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("tail_node", "Tail filter", "d", //Tags:
        "slice",
        "frame",
        "wave",
        "still",
        "window"
    )

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)


    Q_INVOKABLE explicit TailNode(AbstractSession* sess);
    virtual ~TailNode();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    bool isLimited() const;
    int maximumRows() const;

    void setLimited(bool v);
    void setMaximumRows(int v);

    QAbstractItemModel* filteredModel() const;


Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    TailNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TailNode)

};
Q_DECLARE_METATYPE(TailNode*)
