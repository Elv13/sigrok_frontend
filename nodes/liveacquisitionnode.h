#pragma once

#include "abstractnode.h"

#include <memory>

#include <QtCore/QObject>

#include <QtCore/QAbstractItemModel>

#include "sigrokd/sigrokdevice.h"

class AcquisitionModel;

class AcquisitionNodePrivate;

namespace sigrok {
    class HardwareDevice;
}

class Q_DECL_EXPORT AcquisitionNode : public AbstractNode
{
    Q_OBJECT

    REGISTER_META_DATA("acquisition_node", "Live Acquisition", "", /*Tags:*/
        "hardware",
        "sample",
        "sampling"
    )
public:
    Q_PROPERTY(QAbstractItemModel* model READ model NOTIFY modelChanged USER true)
    Q_PROPERTY(SigrokDevice* device WRITE setDevice USER true)
    Q_PROPERTY(bool aboutToClear READ dummy NOTIFY aboutToClear USER true)
    Q_PROPERTY(bool cleared READ dummy NOTIFY cleared USER true)
    Q_PROPERTY(bool clear WRITE clear USER true)
    Q_PROPERTY(bool start WRITE start USER true)
    Q_PROPERTY(bool stop WRITE stop USER true)
    Q_PROPERTY(bool isRunning READ isRunning NOTIFY runningChanged USER true)

    Q_INVOKABLE explicit AcquisitionNode(AbstractSession* sess);

    virtual ~AcquisitionNode();

    virtual void write(QJsonObject &parent) const override;

    void setModel(AcquisitionModel* m);
    AcquisitionModel* aqModel() const;

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
    void modelChanged(QAbstractItemModel*);
    void aboutToClear();
    void cleared();
    void runningChanged(bool);

private:
    AcquisitionNodePrivate* d_ptr;
    Q_DECLARE_PRIVATE(AcquisitionNode)

};

Q_DECLARE_METATYPE(AcquisitionNode*)
