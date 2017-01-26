#pragma once

#include "proxynode.h"

#include <QtCore/QObject>

class MementoNodePrivate;

class Q_DECL_EXPORT MementoNode : public ProxyNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* selectedMemento READ selectedMemento NOTIFY selectedMementoChanged USER true)
    Q_PROPERTY(QAbstractItemModel* lastestMemento READ lastestMemento NOTIFY mementoAdded USER true)
    Q_PROPERTY(TRIGGER trigger WRITE takeMemento USER true)

    Q_INVOKABLE explicit MementoNode(AbstractSession* sess);
    virtual ~MementoNode();

    virtual QString title() const override;
    virtual QWidget* widget() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

    QAbstractItemModel* selectedMemento() const;
    QAbstractItemModel* lastestMemento() const;

    //Mutator
    void takeMemento(bool _);

Q_SIGNALS:
    void selectedMementoChanged(const QAbstractItemModel* m);
    void mementoAdded(const QAbstractItemModel* m);

private:
    MementoNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(MementoNode)

};
Q_DECLARE_METATYPE(MementoNode*)
