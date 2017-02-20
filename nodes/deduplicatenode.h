#pragma once

#include "proxynode.h"

#include <QtCore/QObject>
#include <QtCore/QAbstractItemModel>

class DeduplicateNodePrivate;

class Q_DECL_EXPORT DeduplicateNode : public ProxyNode
{
    Q_OBJECT

    REGISTER_META_DATA("deduplicate_node", "Squash", "", /*Tags:*/
        "merge",
        "deduplicate"
    )
public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)
    Q_PROPERTY(qreal threshold READ threshold WRITE setThreshold NOTIFY thresholdChanged USER true)

    Q_INVOKABLE explicit DeduplicateNode(AbstractSession* sess);
    virtual ~DeduplicateNode();

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    virtual QWidget* widget() const override;

    bool hasExtraColumn() const;
    qreal threshold() const;

    void setThreshold(qreal v);

    QAbstractItemModel* filteredModel() const;

Q_SIGNALS:
    void filteredModelChanged(); //dummy
    void thresholdChanged(qreal);

private:
    DeduplicateNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(DeduplicateNode)

};
Q_DECLARE_METATYPE(DeduplicateNode*)
