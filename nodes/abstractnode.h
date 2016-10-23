#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>

class QWidget;
class QAbstractItemModel;

class AbstractNode : public QObject
{
    Q_OBJECT
public:
    explicit AbstractNode(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~AbstractNode() {}

    virtual QString title() const = 0;
    virtual QString id() const = 0;

    virtual QWidget* widget() const {return nullptr;}

    virtual void write(QJsonObject &parent) const;

Q_SIGNALS:
    void titleChanged(const QString& title);

};
Q_DECLARE_METATYPE(AbstractNode*)
