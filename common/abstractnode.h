#pragma once

#include <QtCore/QObject>
#include <QtCore/QJsonObject>

#include <array>

class QWidget;
#include <QtCore/QAbstractItemModel>

typedef bool TRIGGER;
typedef bool PULSE;

class AbstractSession;

class AbstractNodePrivate;

struct NodeMetaData
{
    const char* name;
    const char* title;
    const char* iconName;
    const std::array<const char*, 10> tags;
};

// The values need to be both polymorphic and static
// After messing with template code, I came back to macros because
// constexpr template and losange inheritance are worst than macros
#define REGISTER_META_DATA(name_, title_, iconName_, ...)public:\
    constexpr static const NodeMetaData nodeMetaData() {\
         return {name_, title_, iconName_, {__VA_ARGS__}}; }\
    virtual QString title() const override {\
    static QString s(nodeMetaData().title); return s;}\
    virtual QString id() const override {\
    static QString s(nodeMetaData().name); return s;}\
    virtual QByteArray iconName() const override {return nodeMetaData().iconName;}\
    virtual QStringList searchTags() const override {return{};}private:

class Q_DECL_EXPORT AbstractNode : public QObject
{
    Q_OBJECT
public:
    enum class Mode {
        PROPERTY,
        MODEL,
    };

    enum class NotifyPriority {
        NORMAL,
        WARNING,
        ERROR,
        CRITICAL,
    };

    explicit AbstractNode(AbstractSession* sess);
    virtual ~AbstractNode();

    // These accessors are auto-generated from the macro
    virtual QString title() const = 0;
    virtual QString id() const = 0;
    virtual QByteArray iconName() const{return{};};
    virtual QStringList searchTags() const;

    void setUid(const QString& uid);
    QString uid() const;

    virtual QWidget* widget() const {return nullptr;}
    virtual Mode mode() const {return Mode::PROPERTY;}

    virtual QAbstractItemModel* sourceModel() const {return nullptr;}
    virtual QAbstractItemModel* sinkModel() const {return nullptr;}

    virtual bool createSourceSocket(const QString& name);
    virtual bool createSinkSocket(const QString& name);

    virtual QString remoteObjectName() const;
    virtual QString remoteModelName() const;
    virtual QString remoteWidgetType() const;

    virtual void write(QJsonObject &parent) const;
    virtual void read(const QJsonObject &parent);


Q_SIGNALS:
    void titleChanged(const QString& title);
    void notify(
        const QString& message,
        bool system = false,
        NotifyPriority p = NotifyPriority::NORMAL
    );

protected:
    AbstractSession* session() const;

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

