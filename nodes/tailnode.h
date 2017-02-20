#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class TailNodePrivate;

class Q_DECL_EXPORT TailNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* filteredModel READ filteredModel NOTIFY filteredModelChanged USER true)


    Q_INVOKABLE explicit TailNode(AbstractSession* sess);
    virtual ~TailNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    bool isLimited() const;
    int maximumRows() const;

    void setLimited(bool v);
    void setMaximumRows(int v);

    QAbstractItemModel* filteredModel() const;

    virtual QStringList searchTags() const override;

Q_SIGNALS:
    void filteredModelChanged(); //dummy

private:
    TailNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(TailNode)

};
Q_DECLARE_METATYPE(TailNode*)
