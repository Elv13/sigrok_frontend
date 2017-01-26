#pragma once

#include "abstractnode.h"

#include <memory>

#include <QtCore/QObject>

#include <QtCore/QAbstractItemModel>

#include "sigrokd/sigrokdevice.h"

class AquisitionModel;

class AquisitionNodePrivate;

namespace sigrok {
    class HardwareDevice;
}

class Q_DECL_EXPORT AquisitionNode : public AbstractNode
{
    Q_OBJECT

public:
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY modelChanged USER true)
    Q_PROPERTY(SigrokDevice* device WRITE setDevice USER true)
    Q_PROPERTY(bool aboutToClear READ dummy NOTIFY aboutToClear USER true)
    Q_PROPERTY(bool cleared READ dummy NOTIFY cleared USER true)
    Q_PROPERTY(bool clear WRITE clear USER true)
    Q_PROPERTY(bool start WRITE start USER true)
    Q_PROPERTY(bool stop WRITE stop USER true)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged USER true)

    Q_INVOKABLE explicit AquisitionNode(AbstractSession* sess);

    virtual ~AquisitionNode();

    virtual QString title() const override;

    virtual QString id() const override;

    virtual void write(QJsonObject &parent) const override;

    void setModel(AquisitionModel* m);
    AquisitionModel* aqModel() const;

    bool isRunning() const;

    virtual QAbstractItemModel* model() const;

    bool dummy() const;

    void clear(bool value);
    void start(bool value);
    void stop(bool value);

    SigrokDevice* device() const;
    virtual void setDevice(SigrokDevice* dev);

    virtual QWidget* widget() const override;



Q_SIGNALS:
    void modelChanged(); //Dummy
    void aboutToClear();
    void cleared();
    void runningChanged(bool);

private:
    AquisitionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(AquisitionNode)

};

Q_DECLARE_METATYPE(AquisitionNode*)
