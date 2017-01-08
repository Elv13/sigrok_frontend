#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>

class QWidget;
#include <QtCore/QAbstractItemModel>

typedef bool TRIGGER;
typedef bool PULSE;

class AbstractNodePrivate;

class Q_DECL_EXPORT AbstractNode : public QObject
{
    Q_OBJECT
public:
    enum class Mode {
        PROPERTY,
        MODEL,
    };

    explicit AbstractNode(QObject* parent = nullptr);
    virtual ~AbstractNode();
//     virtual ~AbstractNode() {}

    virtual QString title() const = 0;
    virtual QString id() const = 0;

    void setUid(const QString& uid);
    QString uid() const;

    virtual QWidget* widget() const {return nullptr;}
    virtual Mode mode() const {return Mode::PROPERTY;}

    virtual QAbstractItemModel* sourceModel() const {return nullptr;}
    virtual QAbstractItemModel* sinkModel() const {return nullptr;}

    virtual bool createSocket(const QString& name);

    virtual QString remoteObjectName() const;
    virtual QString remoteModelName() const;
    virtual QString remoteWidgetType() const;

    virtual void write(QJsonObject &parent) const;
    virtual void read(const QJsonObject &parent);

Q_SIGNALS:
    void titleChanged(const QString& title);

private:
    AbstractNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(AbstractNode)

};
Q_DECLARE_METATYPE(AbstractNode*)
Q_DECLARE_METATYPE(TRIGGER)
// Q_DECLARE_METATYPE(PULSE)
Q_DECLARE_METATYPE(QAbstractItemModel*)

#ifndef Q_DECLARE_STREAM_METATYPE
#define Q_DECLARE_STREAM_METATYPE(T) \
QDataStream &operator<<(QDataStream &s, const T);\
QDataStream &operator<<(QDataStream &s, const T) { return s; }\
QDataStream &operator>>(QDataStream &s, const T);\
QDataStream &operator>>(QDataStream &s, const T) { return s; }\
static int ___ = ([]()->int{\
    qRegisterMetaType<T>(#T);\
    qRegisterMetaTypeStreamOperators<T>(#T);\
    return 0;\
})();
#endif

