#pragma once

#include "abstractnode.h"

#include <QtCore/QObject>

class SequenceNodePrivate;

class Q_DECL_EXPORT SequenceNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("sequence_node", "Sequence", "", /*Tags:*/
        "automation",
        "set",
        "loop",
        "names",
        "tags",
        "label"
    )

public:
    Q_PROPERTY(bool forward WRITE forward USER true)
    Q_PROPERTY(bool backward WRITE backward USER true)
    Q_PROPERTY(bool reset WRITE reset USER true)
    Q_PROPERTY(QString next READ next NOTIFY nextChanged USER true)
    Q_PROPERTY(QString previous READ previous NOTIFY previousChanged USER true)
    Q_PROPERTY(QString add WRITE add USER true)
    Q_PROPERTY(bool looping READ isLooping WRITE setLooping NOTIFY loopingChanged USER true)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrent NOTIFY currentIndexChanged USER true)
    Q_PROPERTY(bool hasEnded READ hasEnded NOTIFY endedChanged USER true)

    Q_INVOKABLE explicit SequenceNode(AbstractSession* sess);
    virtual ~SequenceNode();

    virtual QWidget* widget() const override;

    void forward(bool);
    void backward(bool);
    void reset(bool);

    QString next() const;
    QString previous() const;
    void add(const QString& v);

    bool isLooping() const;
    void setLooping(bool);
    bool hasEnded() const;

    int currentIndex() const;
    void setCurrent(int);

    virtual void write(QJsonObject &parent) const override;
    virtual void read(const QJsonObject &parent) override;

Q_SIGNALS:
    void currentIndexChanged(int);
    void previousChanged(const QString&);
    void nextChanged(const QString&);
    void loopingChanged(bool);
    void endedChanged(bool);

private:
    SequenceNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(SequenceNode)

};
Q_DECLARE_METATYPE(SequenceNode*)
