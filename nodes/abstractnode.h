#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>

class QWidget;
#include <QtCore/QAbstractItemModel>

typedef bool TRIGGER;
typedef bool PULSE;

class AbstractNode : public QObject
{
    Q_OBJECT
public:
    enum class Mode {
        PROPERTY,
        MODEL,
    };

    explicit AbstractNode(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~AbstractNode() {}

    virtual QString title() const = 0;
    virtual QString id() const = 0;

    virtual QWidget* widget() const {return nullptr;}
    virtual Mode mode() const {return Mode::PROPERTY;}

    virtual QAbstractItemModel* sourceModel() const {return nullptr;}
    virtual QAbstractItemModel* sinkModel() const {return nullptr;}

    virtual void write(QJsonObject &parent) const;
    virtual void read(const QJsonObject &parent);

Q_SIGNALS:
    void titleChanged(const QString& title);

};
Q_DECLARE_METATYPE(AbstractNode*)
Q_DECLARE_METATYPE(TRIGGER)
// Q_DECLARE_METATYPE(PULSE)
Q_DECLARE_METATYPE(QAbstractItemModel*)
