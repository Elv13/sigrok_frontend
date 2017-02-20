#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class HeadNodePrivate;

class Q_DECL_EXPORT HeadNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("head_node", "Head filter", "", /*Tags:*/ "")
public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)


    Q_INVOKABLE explicit HeadNode(AbstractSession* sess);
    virtual ~HeadNode();

    virtual QWidget* widget() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    QAbstractItemModel* filteredModel() const;

    bool isLimited() const;
    int maximumRows() const;

    void setLimited(bool v);
    void setMaximumRows(int v);

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    HeadNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(HeadNode)

};
Q_DECLARE_METATYPE(HeadNode*)
