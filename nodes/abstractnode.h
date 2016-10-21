#pragma once

#include <QtCore/QObject>

class QWidget;
class QAbstractItemModel;

class AbstractNode : public QObject
{
    Q_OBJECT
public:
    explicit AbstractNode(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~AbstractNode() {}

    virtual QString title() const = 0;

    virtual QWidget* widget() const {return nullptr;}

};
Q_DECLARE_METATYPE(AbstractNode*)
